// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Data/DrunkardDataAsset.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/CombatReceiver.h"
#include "Items/WeaponBase.h"


namespace
{
	// DrunkardData 미지정 시 폴백 (수치 튜닝은 데이터 에셋에서, 값은 에셋 기본값과 동일하게 유지)
	constexpr float FallbackEnterDuration = 2.f;
	constexpr float FallbackChaseDuration = 10.f;
	constexpr float FallbackExitTimeout = 10.f;
	constexpr float FallbackCaptureKnockbackForce = 300.f;
	constexpr float FallbackCaptureKnockbackUpForce = 200.f;
	constexpr float FallbackCaptureKnockbackForceNoInstrument = 300.f;
	constexpr float FallbackCaptureKnockbackUpForceNoInstrument = 200.f;
	constexpr float FallbackDiveTimeout = 10.f;
}

UDrunkardStateComponent::UDrunkardStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDrunkardStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UDrunkardStateComponent::BeginEntering()
{
	if (!HasAuthority()) return;

	SetState(EDrunkardState::Entering);

	const UDrunkardDataAsset* Data = GetData();
	const float EnterDuration = Data ? Data->EnterDuration : FallbackEnterDuration;
	GetWorld()->GetTimerManager().SetTimer(
		EnterTimerHandle,
		this,
		&ThisClass::BeginChasing,
		EnterDuration,
		false
	);
}

void UDrunkardStateComponent::BeginChasing()
{
	if (!HasAuthority()) return;

	SetTarget(PickTargetByRankWeight(nullptr));
	SetState(EDrunkardState::Chasing);

	// 지속시간 계산 시작 시점 = 문을 완전히 나온 시점
	const UDrunkardDataAsset* Data = GetData();
	const float ChaseDuration = Data ? Data->ChaseDuration : FallbackChaseDuration;
	GetWorld()->GetTimerManager().SetTimer(
		ChaseTimerHandle,
		this,
		&ThisClass::BeginExiting,
		ChaseDuration,
		false
	);
}

void UDrunkardStateComponent::BeginExiting()
{
	if (!HasAuthority()) return;

	GetWorld()->GetTimerManager().ClearTimer(ChaseTimerHandle);
	SetTarget(nullptr);
	SetState(EDrunkardState::Exiting);

	// 제한 시간 안에 문에 도달해 소멸하지 못하면(경로 막힘, 도달 판정 실패 등) 강제 소멸.
	// 스포너의 재스폰이 NPC 소멸에 걸려 있어, 여기서 끼면 기믹 전체가 영구 정지한다
	const UDrunkardDataAsset* Data = GetData();
	GetWorld()->GetTimerManager().SetTimer(
		ExitTimerHandle,
		this,
		&ThisClass::HandleExitTimeout,
		Data ? Data->ExitTimeout : FallbackExitTimeout,
		false
	);
}

void UDrunkardStateComponent::HandleExitTimeout()
{
	if (!HasAuthority() || !GetOwner()) return;

	UE_LOG(LogDrunkard, Warning, TEXT("%s 퇴장 제한 시간 초과 — 문 도달 실패로 강제 소멸 (문 위치/내비메시/도달 판정 반경 확인 필요)"), *GetOwner()->GetName());
	DespawnOwner();
}

void UDrunkardStateComponent::DespawnOwner()
{
	if (!HasAuthority()) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 스폰 시 자동 생성된 AIController는 폰이 죽어도 남는다. BT 틱 중 즉시 파괴는 위험하므로 지연 파괴
	if (const APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		if (AController* Controller = OwnerPawn->GetController())
		{
			Controller->SetLifeSpan(0.1f);
		}
	}
	Owner->Destroy();
}

void UDrunkardStateComponent::RequestTargetChange()
{
	if (!HasAuthority() || State != EDrunkardState::Chasing) return;

	if (ADefaultTromboneCharacter* NewTarget = PickRandomTarget(Target.Get()))
	{
		SetTarget(NewTarget);
	}
}

void UDrunkardStateComponent::HandleCaptureContact(AActor* OtherActor)
{
	if (!HasAuthority() || State != EDrunkardState::Chasing) return;

	// 피격 경직 중에는 포획 불가
	const ADrunkardNPC* OwnerNPC = Cast<ADrunkardNPC>(GetOwner());
	if (OwnerNPC && OwnerNPC->IsStun()) return;

	ADefaultTromboneCharacter* TargetCharacter = Target.Get();
	if (!TargetCharacter || OtherActor != TargetCharacter) return;

	bool bHasInstrument = false;
	if (const UEquipmentComponent* Equipment = TargetCharacter->GetEquipmentComponent())
	{
		if (const AWeaponBase* Weapon = Cast<AWeaponBase>(Equipment->GetItemInSlot(EEquipmentSlotType::Weapon)))
		{
			bHasInstrument = Weapon->GetWeaponType() != EWeaponType::Headbutt;
		}
	}

	const UDrunkardDataAsset* Data = GetData();

	FHitData HitData;
	HitData.HitDirection = (TargetCharacter->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D();
	HitData.HitInstigator = EHitInstigatorType::Drunkard;
	HitData.HitInstigatorActor = GetOwner();
	HitData.HitReaction = bHasInstrument ? EHitReactionType::Ragdoll : EHitReactionType::KnockbackOnly;

	if (bHasInstrument)
	{
		HitData.KnockbackForce = Data ? Data->CaptureKnockbackForce : FallbackCaptureKnockbackForce;
		HitData.KnockbackUpForce = Data ? Data->CaptureKnockbackUpForce : FallbackCaptureKnockbackUpForce;
	}
	else
	{
		HitData.KnockbackForce = Data ? Data->CaptureKnockbackForceNoInstrument : FallbackCaptureKnockbackForceNoInstrument;
		HitData.KnockbackUpForce = Data ? Data->CaptureKnockbackUpForceNoInstrument : FallbackCaptureKnockbackUpForceNoInstrument;
	}

	const bool bApplied = ICombatReceiver::Execute_OnHitReceived(TargetCharacter, HitData);
	if (!bApplied)
	{
		RequestTargetChange();
		return;
	}

	if (bHasInstrument)
	{
		BeginDiving();
	}
	else
	{
		RequestTargetChange();
	}
}

void UDrunkardStateComponent::BeginDiving()
{
	if (!HasAuthority()) return;

	GetWorld()->GetTimerManager().ClearTimer(ChaseTimerHandle);
	SetTarget(nullptr);
	SetState(EDrunkardState::Diving);

	if (ADrunkardNPC* OwnerNPC = Cast<ADrunkardNPC>(GetOwner()))
	{
		OwnerNPC->BeginDive();
	}

	// 제한 시간 후 강제 퇴장
	const UDrunkardDataAsset* Data = GetData();
	GetWorld()->GetTimerManager().SetTimer(
		DiveTimerHandle,
		this,
		&ThisClass::HandleDiveFinished,
		Data ? Data->DiveTimeout : FallbackDiveTimeout,
		false
	);
}

void UDrunkardStateComponent::HandleDiveFinished()
{
	if (!HasAuthority() || State != EDrunkardState::Diving) return;

	GetWorld()->GetTimerManager().ClearTimer(DiveTimerHandle);
	BeginExiting();
}

void UDrunkardStateComponent::SetState(const EDrunkardState NewState)
{
	if (State == NewState) return;

	State = NewState;
	OnStateChanged.Broadcast(State);
}

void UDrunkardStateComponent::SetTarget(ADefaultTromboneCharacter* NewTarget)
{
	if (Target.Get() == NewTarget) return;

	Target = NewTarget;
	OnTargetChanged.Broadcast(NewTarget);
}

float UDrunkardStateComponent::GetRemainingEnterTime() const
{
	return GetWorld() ? GetWorld()->GetTimerManager().GetTimerRemaining(EnterTimerHandle) : -1.f;
}

float UDrunkardStateComponent::GetRemainingChaseTime() const
{
	return GetWorld() ? GetWorld()->GetTimerManager().GetTimerRemaining(ChaseTimerHandle) : -1.f;
}

ADefaultTromboneCharacter* UDrunkardStateComponent::PickTargetByRankWeight(const ADefaultTromboneCharacter* Exclude) const
{
	const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GameState) return nullptr;

	// 후보 수집 후 점수 내림차순 정렬 = 순위
	struct FCandidate
	{
		ADefaultTromboneCharacter* Character;
		float Score;
	};
	TArray<FCandidate> Candidates;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (!PlayerState) continue;

		ADefaultTromboneCharacter* Character = Cast<ADefaultTromboneCharacter>(PlayerState->GetPawn());
		if (!Character || Character == Exclude) continue;

		Candidates.Add({ Character, PlayerState->GetScore() });
	}
	if (Candidates.IsEmpty()) return nullptr;

	Candidates.Sort([](const FCandidate& A, const FCandidate& B) { return A.Score > B.Score; });

	// 순위 가중치 랜덤 (순위 높을수록 확률↑). 배열보다 낮은 순위는 마지막 가중치 사용
	const UDrunkardDataAsset* Data = GetData();
	const TArray<float>* Weights = (Data && !Data->TargetRankWeights.IsEmpty()) ? &Data->TargetRankWeights : nullptr;

	auto GetRankWeight = [Weights](const int32 RankIndex) -> float
	{
		if (!Weights) return 1.f;
		const int32 Index = FMath::Min(RankIndex, Weights->Num() - 1);
		return FMath::Max(0.f, (*Weights)[Index]);
	};

	float TotalWeight = 0.f;
	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		TotalWeight += GetRankWeight(i);
	}
	if (TotalWeight <= 0.f)
	{
		return Candidates[FMath::RandRange(0, Candidates.Num() - 1)].Character;
	}

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		Roll -= GetRankWeight(i);
		if (Roll <= 0.f)
		{
			return Candidates[i].Character;
		}
	}
	return Candidates.Last().Character;
}

ADefaultTromboneCharacter* UDrunkardStateComponent::PickRandomTarget(const ADefaultTromboneCharacter* Exclude) const
{
	const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GameState) return nullptr;

	TArray<ADefaultTromboneCharacter*> Candidates;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (!PlayerState) continue;

		ADefaultTromboneCharacter* Character = Cast<ADefaultTromboneCharacter>(PlayerState->GetPawn());
		if (!Character || Character == Exclude) continue;

		Candidates.Add(Character);
	}
	if (Candidates.IsEmpty()) return nullptr;

	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

const UDrunkardDataAsset* UDrunkardStateComponent::GetData() const
{
	const ADrunkardNPC* NPC = Cast<ADrunkardNPC>(GetOwner());
	return NPC ? NPC->GetDrunkardData() : nullptr;
}

bool UDrunkardStateComponent::HasAuthority() const
{
	return GetOwner() && GetOwner()->HasAuthority();
}
