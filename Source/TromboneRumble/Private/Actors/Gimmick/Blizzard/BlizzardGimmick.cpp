// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Blizzard/BlizzardGimmick.h"
#include "Actors/Gimmick/Blizzard/BlizzardShelter.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/TromboneCharacterBase.h"
#include "Interfaces/CombatReceiver.h"
#include "Utilities/Defines.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

ABlizzardGimmick::ABlizzardGimmick()
{
	// Active 동안 서버에서만 켜서 프레임 단위 밀기에 사용
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	// 맵 전역 이벤트이므로 위치 기반 relevancy 컬링으로 클라가 상태 복제를 놓치지 않게 함
	bAlwaysRelevant = true;

	GimmickType = EGimmickType::Blizzard;
}

void ABlizzardGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BlizzardState);
	DOREPLIFETIME(ThisClass, WindDirection);
}

void ABlizzardGimmick::Activate()
{
	const bool bWasActive = IsActive();

	Super::Activate();

	if (!bWasActive && HasAuthority())
	{
		ScheduleNextBlizzard();
	}
}

void ABlizzardGimmick::Deactivate()
{
	if (HasAuthority())
	{
		SetActorTickEnabled(false);
		PushTargets.Reset();
		RemoveAllSlows();
		ExposureTimeMap.Empty();
		SetState(EBlizzardState::Idle);
	}

	// Super가 bIsActive=false + ClearAllTimersForObject 로 3개 타이머를 정리한다.
	Super::Deactivate();
}

void ABlizzardGimmick::ScheduleNextBlizzard()
{
	if (!HasAuthority()) return;

	const float NextInterval = FMath::RandRange(MinIntervalSeconds, MaxIntervalSeconds);

	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	GetWorldTimerManager().SetTimer(
		ScheduleTimerHandle,
		this,
		&ThisClass::StartWarning,
		NextInterval,
		false
	);
}

void ABlizzardGimmick::StartWarning()
{
	if (!HasAuthority()) return;

	// 방향을 먼저 확정해 SetState 와 같은 번치로 복제되게 함
	const FVector Axis = WindAxis.GetSafeNormal2D();
	WindDirection = Axis * (FMath::RandBool() ? 1.f : -1.f);

	SetState(EBlizzardState::Warning);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&ThisClass::StartBlizzard,
		WarningDuration,
		false
	);
}

void ABlizzardGimmick::StartBlizzard()
{
	if (!HasAuthority()) return;

	ExposureTimeMap.Empty();
	PushTargets.Reset();
	GatherShelters();

	SetState(EBlizzardState::Active);
	SetActorTickEnabled(true);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&ThisClass::EndBlizzard,
		ActiveDuration,
		false
	);

	GetWorldTimerManager().ClearTimer(ExposureTimerHandle);
	GetWorldTimerManager().SetTimer(
		ExposureTimerHandle,
		this,
		&ThisClass::TickExposure,
		ExposureCheckInterval,
		true
	);
}

void ABlizzardGimmick::EndBlizzard()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(ExposureTimerHandle);

	SetActorTickEnabled(false);
	PushTargets.Reset();
	RemoveAllSlows();
	ExposureTimeMap.Empty();

	SetState(EBlizzardState::Idle);

	ScheduleNextBlizzard();
}

void ABlizzardGimmick::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || BlizzardState != EBlizzardState::Active) return;

	const FVector WindDir = WindDirection;
	if (WindDir.IsNearlyZero()) return;

	// 바람을 위치 오프셋으로 적용해 플레이어 이동과 "합성"한다 (Velocity를 건드리지 않음).
	// 속도를 직접 수정하면 CMC 브레이킹/최대속도 클램프와 싸우게 되어,
	// 정지자를 밀 수 있는 세기는 반드시 이동 입력의 저항도 압도해버린다.
	// 위치 오프셋은 CMC 계산 결과 위에 얹히므로: 정지 시 PushSpeed 로 밀리고,
	// 역풍 이동 시 (이동속도 - PushSpeed) 로 저항하며 전진할 수 있다.
	for (const TWeakObjectPtr<ADefaultTromboneCharacter>& TargetPtr : PushTargets)
	{
		ADefaultTromboneCharacter* Target = TargetPtr.Get();
		if (!IsValid(Target)) continue;
		if (Target->IsRagdoll()) continue;  // 래그돌 굴리기는 TickExposure 가 담당

		const UCharacterMovementComponent* Move = Target->GetCharacterMovement();
		if (!Move) continue;

		// 기준 속도 = 현재 걷기 속도. MaxWalkSpeed 에는 슬로우/버프/악기장착이 이미 반영돼 있고,
		// 스프린트 중에는 SprintSpeed 로 바뀌므로 Walk/Sprint 비로 걷기 기준 환산
		// (바람이 스프린트에 비례 강화되지 않아야 달리기가 확실한 탈출 수단이 된다)
		float ReferenceSpeed = Move->MaxWalkSpeed;
		const UCharacterDataAsset* Data = Target->GetCharacterDataAsset();
		if (Data && Target->IsSprinting() && Data->SprintSpeed > KINDA_SMALL_NUMBER)
		{
			ReferenceSpeed *= Data->WalkSpeed / Data->SprintSpeed;
		}

		const float PushSpeed = ReferenceSpeed * WindPushRatio;

		// sweep=true: 벽/다른 캐릭터를 뚫고 밀리지 않게 함
		Target->AddActorWorldOffset(WindDir * PushSpeed * DeltaSeconds, true);
	}
}

void ABlizzardGimmick::SetState(EBlizzardState NewState)
{
	if (!HasAuthority()) return;
	if (BlizzardState == NewState) return;

	BlizzardState = NewState;
	OnRep_BlizzardState();  // 리슨 서버 호스트에서도 연출 이벤트가 발화되도록 수동 호출
	ForceNetUpdate();
}

void ABlizzardGimmick::OnRep_BlizzardState()
{
	OnBlizzardStateChanged(BlizzardState, WindDirection);
}

void ABlizzardGimmick::TickExposure()
{
	if (!HasAuthority() || BlizzardState != EBlizzardState::Active) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 무효 위크포인터 정리
	for (auto It = ExposureTimeMap.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid()) It.RemoveCurrent();
	}

	TArray<ADefaultTromboneCharacter*> Characters;
	for (TActorIterator<ADefaultTromboneCharacter> It(World); It; ++It)
	{
		ADefaultTromboneCharacter* Character = *It;
		if (!IsValid(Character)) continue;
		Characters.Add(Character);
	}

	const FVector WindDir = WindDirection;
	if (WindDir.IsNearlyZero()) return;

	PushTargets.Reset();

	for (ADefaultTromboneCharacter* Character : Characters)
	{
		// 안전지대 안: 밀림/슬로우/누적 없음, 누적 리셋
		if (IsCharacterInShelter(Character))
		{
			ExposureTimeMap.Add(Character, 0.f);
			RemoveSlow(Character);
			continue;
		}

		// 노출 + 래그돌 중: 누적 0 유지(기상 후 새로 카운트), 슬로우 제거, 바람 방향으로 굴리기
		if (Character->IsRagdoll())
		{
			ExposureTimeMap.Add(Character, 0.f);
			RemoveSlow(Character);
			if (bRollRagdolledCharacters)
			{
				if (USkeletalMeshComponent* Mesh = Character->GetMesh())
				{
					Mesh->AddImpulse(
						WindDir * RagdollRollImpulsePerSecond * ExposureCheckInterval,
						TromboneBones::Pelvis,
						true
					);
				}
			}
			continue;
		}

		// 노출 + 일반: 누적 → 임계 도달 시 래그돌
		const float NewExposure = ExposureTimeMap.FindRef(Character) + ExposureCheckInterval;

		if (NewExposure >= RagdollExposureThreshold)
		{
			TriggerRagdoll(Character);

			// 발동 성공(무적/스턴 게이트 통과) 시에만 누적 리셋. 게이트되면 누적 유지 → 해제 직후 재시도
			if (Character->IsRagdoll())
			{
				ExposureTimeMap.Add(Character, 0.f);
				RemoveSlow(Character);
				continue;
			}
		}

		ExposureTimeMap.Add(Character, NewExposure);

		// 밀어내기는 Tick 에서 매 프레임 속도 보충으로 처리 (펄스 임펄스는 Walking 브레이킹에 씹힘)
		PushTargets.Add(Character);

		// 역풍 이동 시 슬로우 (이동 의도 = 가속도 기준, 속도는 밀림으로 오염됨)
		bool bMovingIntoWind = false;
		if (const UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			const FVector Accel = Move->GetCurrentAcceleration();
			if (!Accel.IsNearlyZero())
			{
				const float Dot = FVector::DotProduct(Accel.GetSafeNormal2D(), WindDir);
				const bool bHasSlow = ActiveSlowEffects.Contains(Character);
				// 히스테리시스: 적용/해제 임계를 분리해 방향 전환 시 깜빡임 방지
				if (!bHasSlow && Dot < SlowApplyDotThreshold)
				{
					bMovingIntoWind = true;
				}
				else if (bHasSlow && Dot < SlowReleaseDotThreshold)
				{
					bMovingIntoWind = true;
				}
			}
		}

		if (bMovingIntoWind)
		{
			ApplySlow(Character);
		}
		else
		{
			RemoveSlow(Character);
		}
	}
}

void ABlizzardGimmick::GatherShelters()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	Shelters.Reset();
	for (TActorIterator<ABlizzardShelter> It(World); It; ++It)
	{
		if (ABlizzardShelter* Shelter = *It)
		{
			Shelters.Add(Shelter);
		}
	}
}

bool ABlizzardGimmick::IsCharacterInShelter(const ACharacter* Character) const
{
	if (!Character) return false;

	const FVector Loc = Character->GetActorLocation();
	for (const TWeakObjectPtr<ABlizzardShelter>& ShelterPtr : Shelters)
	{
		const ABlizzardShelter* Shelter = ShelterPtr.Get();
		if (Shelter && Shelter->IsLocationInside(Loc))
		{
			return true;
		}
	}
	return false;
}

void ABlizzardGimmick::ApplySlow(ADefaultTromboneCharacter* Character)
{
	if (!HasAuthority() || !Character || !BlizzardSlowEffectClass) return;

	// 이미 적용 중이면 스킵
	if (ActiveSlowEffects.Contains(Character)) return;

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC) return;

	// 방어적 중복 방지 (핸들이 유실됐지만 GE는 남아있는 경우)
	FGameplayEffectQuery SlowQuery;
	SlowQuery.EffectDefinition = BlizzardSlowEffectClass;
	if (ASC->GetActiveEffects(SlowQuery).Num() > 0) return;

	const UGameplayEffect* GE = BlizzardSlowEffectClass->GetDefaultObject<UGameplayEffect>();
	if (!GE) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(GE, 1.f, Context);
	if (Handle.IsValid())
	{
		ActiveSlowEffects.Add(Character, Handle);
	}
}

void ABlizzardGimmick::RemoveSlow(ACharacter* Character)
{
	if (!HasAuthority() || !Character) return;

	FActiveGameplayEffectHandle* FoundHandle = ActiveSlowEffects.Find(Character);
	if (!FoundHandle) return;

	if (const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character))
	{
		if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
		{
			if (FoundHandle->IsValid())
			{
				ASC->RemoveActiveGameplayEffect(*FoundHandle);
			}
		}
	}

	ActiveSlowEffects.Remove(Character);
}

void ABlizzardGimmick::RemoveAllSlows()
{
	if (!HasAuthority()) return;

	for (auto& Pair : ActiveSlowEffects)
	{
		ACharacter* Character = Pair.Key.Get();
		if (!Character) continue;

		if (const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character))
		{
			if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
			{
				if (Pair.Value.IsValid())
				{
					ASC->RemoveActiveGameplayEffect(Pair.Value);
				}
			}
		}
	}

	ActiveSlowEffects.Empty();
}

void ABlizzardGimmick::TriggerRagdoll(ATromboneCharacterBase* Character)
{
	if (!HasAuthority() || !Character) return;
	if (!Character->Implements<UCombatReceiver>()) return;

	// FHitData 경유: 캐릭터의 무적/스턴/래그돌 게이트를 존중하고, RagdollComponent 직접 접근을 피함
	FHitData HitData;
	HitData.HitDirection = WindDirection;
	HitData.KnockbackForce = 0.f;
	HitData.HitReaction = EHitReactionType::Ragdoll;
	HitData.HitInstigator = EHitInstigatorType::Blizzard;

	ICombatReceiver::Execute_OnHitReceived(Character, HitData);

	// 발동 성공 시 즉시 굴림 임펄스 (StartRagdoll 이 동기적으로 물리를 켠다)
	if (Character->IsRagdoll())
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			Mesh->AddImpulse(WindDirection * RagdollTriggerRollImpulse, TromboneBones::Pelvis, true);
		}
	}
}
