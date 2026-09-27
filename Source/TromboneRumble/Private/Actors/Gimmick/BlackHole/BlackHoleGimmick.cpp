// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/BlackHole/BlackHoleGimmick.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Data/CharacterDataAsset.h"
#include "Data/Gimmick/BlackHoleGimmickConfig.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/CombatReceiver.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/TromboneLogs.h"

#if !UE_BUILD_SHIPPING
TAutoConsoleVariable<int32> CVarBlackHoleDebug(
	TEXT("Trombone.BlackHole.Debug"),
	0,
	TEXT("1이면 블랙홀의 상태와 두 반경을 화면에 표시"));
#endif

ABlackHoleGimmick::ABlackHoleGimmick()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	GimmickType = EGimmickType::BlackHole;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	InnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InnerSphere"));
	SetRootComponent(InnerSphere);
	InnerSphere->SetSphereRadius(100.f);
	InnerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerSphere->SetGenerateOverlapEvents(false);
	InnerSphere->SetCanEverAffectNavigation(false);
}

void ABlackHoleGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (InnerSphere)
	{
		InnerSphere->SetSphereRadius(GetInnerRadius(), false);
	}

#if !UE_BUILD_SHIPPING
	IConsoleVariable* DebugVariable = CVarBlackHoleDebug.AsVariable();
	DebugVariable->OnChangedDelegate().AddUObject(this, &ThisClass::HandleDebugCVarChanged);
	HandleDebugCVarChanged(DebugVariable);
#endif
}

void ABlackHoleGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	CVarBlackHoleDebug.AsVariable()->OnChangedDelegate().RemoveAll(this);
#endif

	// Deactivate 가 전부 지우지만, Deactivate 없이 파괴될 수도 있다
	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().ClearTimer(CaptureTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void ABlackHoleGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, State);
	DOREPLIFETIME(ThisClass, ActiveStartServerTime);
}

void ABlackHoleGimmick::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (InnerSphere)
	{
		// 콜리전이 꺼져 있고 매 프레임 자라므로 오버랩을 다시 계산할 이유가 없다
		InnerSphere->SetSphereRadius(GetInnerRadius(), false);
	}

	if (HasAuthority() && IsPulling())
	{
		TickPull(DeltaSeconds);
	}

	DebugDraw();
}

void ABlackHoleGimmick::Activate()
{
	const bool bWasActive = IsActive();

	Super::Activate();

	if (!bWasActive && HasAuthority())
	{
		ScheduleNext(GetConfig<UBlackHoleGimmickConfig>().Schedule.PickFirstDelay());
	}
}

void ABlackHoleGimmick::Deactivate()
{
	if (HasAuthority())
	{
		ReleaseAllCaptured(/*bBurst*/ false);
		SetState(EBlackHoleState::Idle);
	}

	// Super clears the timers
	Super::Deactivate();
}

void ABlackHoleGimmick::ForceTrigger()
{
	if (!HasAuthority() || State != EBlackHoleState::Idle) return;

	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	StartWarning();
}

void ABlackHoleGimmick::ScheduleNext(const float Delay)
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	GetWorldTimerManager().SetTimer(ScheduleTimerHandle, this, &ThisClass::StartWarning, FMath::Max(0.01f, Delay), false);
}

void ABlackHoleGimmick::StartWarning()
{
	if (!HasAuthority()) return;

	SetState(EBlackHoleState::Warning);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::StartActive,
		FMath::Max(0.01f, GetConfig<UBlackHoleGimmickConfig>().Schedule.WarningDuration), false);
}

void ABlackHoleGimmick::StartActive()
{
	if (!HasAuthority()) return;

	// 반경 계산의 기준 시각. 상태와 같은 번치로 가도록 SetState 보다 먼저 대입한다
	if (const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		ActiveStartServerTime = GameState->GetServerWorldTimeSeconds();
	}

	SetState(EBlackHoleState::Active);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::StartCollapse,
		GetConfig<UBlackHoleGimmickConfig>().ActiveDuration, false);

	GetWorldTimerManager().ClearTimer(CaptureTimerHandle);
	GetWorldTimerManager().SetTimer(CaptureTimerHandle, this, &ThisClass::CheckCaptures,
		FMath::Max(0.02f, CaptureCheckInterval), true);
}

void ABlackHoleGimmick::StartCollapse()
{
	if (!HasAuthority()) return;

	// 붕괴가 시작되면 더 이상 새로 잡지 않는다. 끌림은 그대로 이어진다
	GetWorldTimerManager().ClearTimer(CaptureTimerHandle);

	SetState(EBlackHoleState::Collapse);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::EndCollapse,
		FMath::Max(0.05f, GetConfig<UBlackHoleGimmickConfig>().CollapseDuration), false);
}

void ABlackHoleGimmick::EndCollapse()
{
	if (!HasAuthority()) return;

	// 소품 컴포넌트가 여기서 방출을 받는다. Idle 전이보다 먼저 쏴야 구독자가 아직 포획 상태를 들고 있다
	OnBlackHoleBurstDelegate.Broadcast(GetActorLocation());

	ReleaseAllCaptured(/*bBurst*/ true);
	SetState(EBlackHoleState::Idle);

	ScheduleNext(GetConfig<UBlackHoleGimmickConfig>().Schedule.PickInterval());
}

void ABlackHoleGimmick::SetState(const EBlackHoleState NewState)
{
	if (!HasAuthority() || State == NewState) return;

	State = NewState;

	OnRep_State();  // 리슨 서버 호스트에서도 구독자 알림과 연출이 돌게 수동 호출
	ForceNetUpdate();
}

void ABlackHoleGimmick::OnRep_State()
{
	HandleStateChanged();
}

void ABlackHoleGimmick::HandleStateChanged()
{
	// 구독자에게 먼저 알린다. "상태가 바뀌었다"는 사실이지 렌더링 단계가 아니라서 데디 서버에서도 나가야 한다
	OnBlackHoleStateChangedDelegate.Broadcast(State);

	UpdateTickEnabled();

	UE_LOG(LogBlackHole, Log, TEXT("[%s] state -> %s"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"),
		*UEnum::GetValueAsString(State));

	if (GetNetMode() == NM_DedicatedServer || !GetWorld() || GetWorld()->bIsTearingDown) return;

	OnBlackHoleStateChanged(State);
}

float ABlackHoleGimmick::GetGrowthAlpha() const
{
	if (State == EBlackHoleState::Collapse) return 1.f;
	if (State != EBlackHoleState::Active) return 0.f;

	const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GameState) return 0.f;

	const float Duration = FMath::Max(0.01f, GetConfig<UBlackHoleGimmickConfig>().ActiveDuration);
	return FMath::Clamp((GameState->GetServerWorldTimeSeconds() - ActiveStartServerTime) / Duration, 0.f, 1.f);
}

float ABlackHoleGimmick::GetInfluenceRadius() const
{
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();
	return FMath::Lerp(Config.InfluenceRadiusStart, Config.InfluenceRadiusEnd, GetGrowthAlpha());
}

float ABlackHoleGimmick::GetInnerRadius() const
{
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();
	return FMath::Lerp(Config.InnerRadiusStart, Config.InnerRadiusEnd, GetGrowthAlpha());
}

bool ABlackHoleGimmick::IsInsideInfluence(const FVector& Location) const
{
	return IsPulling() && FVector::DistSquared(Location, GetActorLocation()) <= FMath::Square(GetInfluenceRadius());
}

bool ABlackHoleGimmick::IsInsideInner(const FVector& Location) const
{
	return IsPulling() && FVector::DistSquared(Location, GetActorLocation()) <= FMath::Square(GetInnerRadius());
}

FVector ABlackHoleGimmick::ComputeDesiredVelocity(const FVector& Location, const float BaseSpeed, const bool bHorizontalOnly) const
{
	// 꺼져 있을 때도 반경은 시작값을 돌려주므로, 끌림 여부는 여기서 막는다
	if (!IsPulling()) return FVector::ZeroVector;

	const float Radius = GetInfluenceRadius();

	FVector ToObject = Location - GetActorLocation();
	if (bHorizontalOnly)
	{
		ToObject.Z = 0.f;
	}

	const float Distance = ToObject.Size();
	if (Distance > Radius) return FVector::ZeroVector;

	// 중심에 거의 붙으면 방향을 구할 수 없으니 아무 수평 방향이나 쓴다
	const FVector Radial = Distance > 1.f ? ToObject / Distance : FVector::ForwardVector;

	// 위와 반지름의 외적이라 항상 수평이다. 공전은 높이를 바꾸지 않는다
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();
	const FVector Tangent = FVector::CrossProduct(FVector::UpVector, Radial).GetSafeNormal() * (Config.bOrbitClockwise ? -1.f : 1.f);

	const float DistanceAlpha = FMath::Clamp(Distance / FMath::Max(1.f, Radius), 0.f, 1.f);
	FVector Velocity = BaseSpeed * (-Radial * Config.EvalPull(DistanceAlpha) + Tangent * Config.EvalOrbit(DistanceAlpha));

	if (!bHorizontalOnly)
	{
		Velocity.Z *= Config.VerticalPullScale;
	}

	return Velocity;
}

FVector ABlackHoleGimmick::ComputeSteerAccel(const FVector& Location, const FVector& CurrentVelocity, const float SpeedScale) const
{
	const FVector Desired = ComputeDesiredVelocity(Location, GetConfig<UBlackHoleGimmickConfig>().PhysicsReferenceSpeed * SpeedScale, false);
	if (Desired.IsNearlyZero()) return FVector::ZeroVector;

	return SteerToward(Desired, CurrentVelocity);
}

FVector ABlackHoleGimmick::ComputeRingVelocity(const FVector& Location, const FBlackHoleRingSlot& Slot) const
{
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();
	const FVector Center = GetActorLocation();

	FVector Flat = Location - Center;
	Flat.Z = 0.f;

	const float Distance = Flat.Size();
	const FVector Radial = Distance > 1.f ? Flat / Distance : FVector::ForwardVector;
	const FVector Tangent = FVector::CrossProduct(FVector::UpVector, Radial) * (Config.bOrbitClockwise ? -1.f : 1.f);

	const float TargetRadius = FMath::Max(1.f, GetInnerRadius() * Config.RingRadiusRatio + Slot.RadiusOffset);

	// 반경과 높이는 스프링으로 붙잡고 회전만 일정한 각속도로 준다.
	// 반경이 스프링이라 안쪽으로 감겨 들어가지 않고 그 자리를 계속 돈다 - 나선과 고리의 차이가 이것이다
	FVector Velocity = -Radial * (Distance - TargetRadius) * Config.RingSpring
		+ Tangent * FMath::DegreesToRadians(Slot.AngularSpeed) * TargetRadius;
	Velocity.Z = (Center.Z + Slot.HeightOffset - Location.Z) * Config.RingSpring;

	return Velocity;
}

FVector ABlackHoleGimmick::ComputeBurstVelocity(const FVector& Location) const
{
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();

	FVector Flat = Location - GetActorLocation();
	Flat.Z = 0.f;

	// 중심과 정확히 겹쳐 있으면 방향이 없다. 아무 수평 방향으로나 내보낸다
	FVector Direction = Flat.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		const FVector Random = FMath::VRand();
		Direction = FVector(Random.X, Random.Y, 0.f).GetSafeNormal();
		if (Direction.IsNearlyZero())
		{
			Direction = FVector::ForwardVector;
		}
	}

	// 위를 섞어 포물선을 만든다. 수평만 주면 바닥을 미끄러진다
	Direction = (Direction * (1.f - Config.BurstUpRatio) + FVector::UpVector * Config.BurstUpRatio).GetSafeNormal();

	return Direction * Config.BurstSpeed;
}

FVector ABlackHoleGimmick::SteerToward(const FVector& DesiredVelocity, const FVector& CurrentVelocity) const
{
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();

	// 원하는 속도와의 차이에 비례해 밀어준다. 서브스테핑이 없어서 틱마다 임펄스를 쌓으면 프레임레이트에 따라 결과가 달라진다
	return ((DesiredVelocity - CurrentVelocity) * Config.SteerGain).GetClampedToMaxSize(Config.MaxSteerAccel);
}

void ABlackHoleGimmick::TickPull(const float DeltaSeconds)
{
	PulledCount = 0;

	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<ATromboneCharacterBase> It(World); It; ++It)
	{
		ATromboneCharacterBase* Character = *It;
		if (!IsValid(Character)) continue;

		// 래그돌은 캡슐이 쓰러진 자리에 남으므로 골반으로 거리를 잰다
		const bool bRagdoll = Character->IsRagdoll();
		const FVector Sample = bRagdoll ? Character->GetPelvisLocation() : Character->GetActorLocation();

		// 포획된 몸은 고리에 묶여 있다. 밖으로 밀려나도 놓지 않고 고리로 되돌린다
		if (!CapturedCharacters.Contains(Character) && !IsInsideInfluence(Sample)) continue;

		++PulledCount;

		if (bRagdoll)
		{
			SteerRagdoll(Character);
		}
		else
		{
			PullCharacter(Character, DeltaSeconds);
		}
	}
}

void ABlackHoleGimmick::PullCharacter(ATromboneCharacterBase* Character, const float DeltaSeconds)
{
	const UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	if (!Move) return;

	// 기준 속도 = 현재 걷기 속도. MaxWalkSpeed 에는 슬로우/버프/악기장착이 이미 반영돼 있고,
	// 스프린트 중에는 SprintSpeed 로 바뀌므로 Walk/Sprint 비로 걷기 기준 환산
	// (끌림이 스프린트에 비례해 강해지지 않아야 달리기가 확실한 탈출 수단이 된다)
	float ReferenceSpeed = Move->MaxWalkSpeed;
	const ADefaultTromboneCharacter* Player = Cast<ADefaultTromboneCharacter>(Character);
	const UCharacterDataAsset* Data = Character->GetCharacterDataAsset();
	if (Player && Data && Player->IsSprinting() && Data->SprintSpeed > KINDA_SMALL_NUMBER)
	{
		ReferenceSpeed *= Data->WalkSpeed / Data->SprintSpeed;
	}

	// 끌림을 위치 오프셋으로 적용해 플레이어 이동과 "합성"한다 (Velocity를 건드리지 않음).
	// 속도를 직접 수정하면 CMC 브레이킹/최대속도 클램프와 싸우게 되어,
	// 정지자를 끌 수 있는 세기는 반드시 이동 입력의 저항도 압도해버린다.
	// 커브의 끌림 비율이 1보다 작은 구간에서는 그만큼 걸어서 빠져나갈 수 있다
	const FVector Velocity = ComputeDesiredVelocity(Character->GetActorLocation(), ReferenceSpeed, true);
	if (Velocity.IsNearlyZero()) return;

	DebugDrawVelocity(Character->GetActorLocation(), Velocity);

	// sweep=true: 벽이나 다른 캐릭터를 뚫고 끌려가지 않게 함
	Character->AddActorWorldOffset(Velocity * DeltaSeconds, true);
}

void ABlackHoleGimmick::SteerRagdoll(ATromboneCharacterBase* Character)
{
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh) return;

	const FVector Pelvis = Character->GetPelvisLocation();

	// 포획된 몸은 고리를 돌고, 아직 안 잡힌 몸은 커브를 따라 끌려온다
	const FBlackHoleRingSlot* Slot = CapturedCharacters.Find(Character);
	const FVector Desired = Slot
		? ComputeRingVelocity(Pelvis, *Slot)
		: ComputeDesiredVelocity(Pelvis, GetConfig<UBlackHoleGimmickConfig>().PhysicsReferenceSpeed, false);

	DebugDrawVelocity(Pelvis, Desired);

	const FVector Accel = SteerToward(Desired, Mesh->GetPhysicsLinearVelocity(TromboneBones::Pelvis));
	if (Accel.IsNearlyZero()) return;

	// 골반 하나만 밀면 나머지 바디가 바닥에 끌려다니며 마찰로 접선 운동이 죽는다.
	// 중력과 같은 방식으로 모든 바디에 같은 가속도를 줘야 몸 전체가 함께 돈다.
	// 클라는 골반 속도를 매 프레임 덮어쓰므로 힘은 서버에서만 준다
	Mesh->AddForceToAllBodiesBelow(Accel, NAME_None, /*bAccelChange*/ true);
}

void ABlackHoleGimmick::CheckCaptures()
{
	if (!HasAuthority() || !IsPulling()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<ATromboneCharacterBase> It(World); It; ++It)
	{
		ATromboneCharacterBase* Character = *It;
		if (!IsValid(Character) || CapturedCharacters.Contains(Character)) continue;

		const bool bRagdoll = Character->IsRagdoll();
		const FVector Sample = bRagdoll ? Character->GetPelvisLocation() : Character->GetActorLocation();
		if (!IsInsideInner(Sample)) continue;

		if (!bRagdoll)
		{
			TriggerRagdoll(Character);

			// 무적이나 스턴에 막히면 이번엔 넘기고 다음 주기에 다시 시도한다
			if (!Character->IsRagdoll()) continue;
		}

		CaptureCharacter(Character);
	}
}

FBlackHoleRingSlot ABlackHoleGimmick::MakeRingSlot() const
{
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();

	// 개체마다 자리를 조금씩 다르게 줘야 여럿이 잡혔을 때 선이 아니라 띠로 보인다
	FBlackHoleRingSlot Slot;
	Slot.RadiusOffset = FMath::FRandRange(-Config.RingRadiusJitter, Config.RingRadiusJitter);
	Slot.HeightOffset = FMath::FRandRange(-Config.RingHeightJitter, Config.RingHeightJitter);
	Slot.AngularSpeed = FMath::Max(0.f, Config.RingAngularSpeed + FMath::FRandRange(-Config.RingAngularSpeedJitter, Config.RingAngularSpeedJitter));

	return Slot;
}

void ABlackHoleGimmick::CaptureCharacter(ATromboneCharacterBase* Character)
{
	const FBlackHoleRingSlot Slot = MakeRingSlot();

	// 공전 중에는 골반이 계속 움직여 스스로 일어나지 않지만, 잡고 있다는 계약을 명시한다
	if (UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent())
	{
		Ragdoll->SetAutoGetUpEnabled(false);
	}

	// 중력과 바닥 마찰이 남아 있으면 접선 운동이 계속 갉여서 고리가 안 만들어진다.
	// 힘으로 상쇄하지 않고 아예 끈다. 놓아줄 때 다시 켠다
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		Mesh->SetEnableGravity(false);
	}

	CapturedCharacters.Add(Character, Slot);

	UE_LOG(LogBlackHole, Log, TEXT("Captured %s (반경 %+.0f, 높이 %+.0f, %.0f도/초)"),
		*Character->GetName(), Slot.RadiusOffset, Slot.HeightOffset, Slot.AngularSpeed);
}

void ABlackHoleGimmick::TriggerRagdoll(ATromboneCharacterBase* Character)
{
	if (!Character->Implements<UCombatReceiver>()) return;

	// FHitData 경유: 캐릭터의 무적/스턴/래그돌 게이트를 존중하고 RagdollComponent 직접 접근을 피함.
	// 넉백은 0이라 기존 속도를 그대로 들고 쓰러지고, 그 뒤는 공전이 맡는다
	FHitData HitData;
	HitData.HitDirection = (GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
	HitData.KnockbackForce = 0.f;
	HitData.KnockbackUpForce = 0.f;
	HitData.HitReaction = EHitReactionType::Ragdoll;
	HitData.HitInstigator = EHitInstigatorType::BlackHole;

	ICombatReceiver::Execute_OnHitReceived(Character, HitData);
}

void ABlackHoleGimmick::ReleaseAllCaptured(const bool bBurst)
{
	int32 BurstCount = 0;

	for (const TPair<TWeakObjectPtr<ATromboneCharacterBase>, FBlackHoleRingSlot>& Pair : CapturedCharacters)
	{
		ATromboneCharacterBase* Character = Pair.Key.Get();
		if (!IsValid(Character)) continue;

		USkeletalMeshComponent* Mesh = Character->GetMesh();
		const bool bRagdoll = Character->IsRagdoll();

		if (bBurst)
		{
			const FVector Sample = bRagdoll ? Character->GetPelvisLocation() : Character->GetActorLocation();
			const FVector BurstVelocity = ComputeBurstVelocity(Sample);

			if (bRagdoll && Mesh)
			{
				// 끌 때와 같은 이유로 모든 바디에. 속도 변화라 무거운 바디도 같은 속도로 나간다.
				// 일회성이라 조향과 달리 임펄스를 쓴다
				Mesh->AddImpulseToAllBodiesBelow(BurstVelocity, NAME_None, /*bVelChange*/ true);
			}
			else
			{
				// 무적이나 스턴에 막혀 래그돌 없이 잡혀 있던 경우. 캡슐을 직접 띄운다
				Character->LaunchCharacter(BurstVelocity, true, true);
			}

			// 한 프레임짜리 사건이라 잠깐 남겨야 방향을 눈으로 쫓을 수 있다
			DebugDrawVelocity(Sample, BurstVelocity, 2.f);
			++BurstCount;
		}

		// 중력은 임펄스 뒤에 켠다. 붕괴 프레임의 한 스텝을 중력 없이 날지 않도록
		if (Mesh)
		{
			Mesh->SetEnableGravity(true);
		}

		if (UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent())
		{
			Ragdoll->SetAutoGetUpEnabled(true);
		}
	}

	if (bBurst)
	{
		UE_LOG(LogBlackHole, Log, TEXT("붕괴 방출: %d명 (속도 %.0f)"), BurstCount, GetConfig<UBlackHoleGimmickConfig>().BurstSpeed);
	}

	CapturedCharacters.Reset();
}

void ABlackHoleGimmick::DebugDrawVelocity(const FVector& From, const FVector& Velocity, const float LifeTime) const
{
#if !UE_BUILD_SHIPPING
	if (CVarBlackHoleDebug.GetValueOnGameThread() == 0 || Velocity.IsNearlyZero()) return;

	const UWorld* World = GetWorld();
	if (!World) return;

	// 0.5초 뒤 위치까지 그린다. 접선 성분이 실제로 있는지 한눈에 보인다
	DrawDebugDirectionalArrow(World, From, From + Velocity * 0.5f, 30.f, FColor::Yellow, false, LifeTime, 0, 2.f);
#endif
}

void ABlackHoleGimmick::UpdateTickEnabled()
{
	bool bDebug = false;
#if !UE_BUILD_SHIPPING
	bDebug = CVarBlackHoleDebug.GetValueOnGameThread() != 0;
#endif

	SetActorTickEnabled(IsPulling() || bDebug);
}

void ABlackHoleGimmick::HandleDebugCVarChanged(IConsoleVariable* Variable)
{
	UpdateTickEnabled();
}

void ABlackHoleGimmick::DebugDraw() const
{
#if !UE_BUILD_SHIPPING
	if (CVarBlackHoleDebug.GetValueOnGameThread() == 0) return;

	const UWorld* World = GetWorld();
	if (!World) return;

	const FVector Center = GetActorLocation();
	DrawDebugSphere(World, Center, GetInfluenceRadius(), 24, FColor::Cyan, false, -1.f, 0, 2.f);
	DrawDebugSphere(World, Center, GetInnerRadius(), 16, FColor::Red, false, -1.f, 0, 2.f);

	// 포획된 것들이 올라타는 고리. 축 두 개를 넘겨 수평으로 눕힌다
	const float RingRadius = GetInnerRadius() * GetConfig<UBlackHoleGimmickConfig>().RingRadiusRatio;
	DrawDebugCircle(World, Center, RingRadius, 32, FColor::Green, false, -1.f, 0, 2.f,
		FVector::ForwardVector, FVector::RightVector, false);

	if (!GEngine) return;

	const FTimerHandle& Handle = (State == EBlackHoleState::Idle) ? ScheduleTimerHandle : PhaseTimerHandle;
	const float Seconds = GetWorldTimerManager().GetTimerRemaining(Handle);
	const FString Remaining = Seconds >= 0.f ? FString::Printf(TEXT("%.1fs"), Seconds) : TEXT("타이머 없음");

	// 포획된 캐릭터의 골반에 점을 찍어 누가 잡혀 있는지 보여준다
	for (const TPair<TWeakObjectPtr<ATromboneCharacterBase>, FBlackHoleRingSlot>& Pair : CapturedCharacters)
	{
		if (const ATromboneCharacterBase* Captured = Pair.Key.Get())
		{
			DrawDebugPoint(World, Captured->GetPelvisLocation(), 20.f, FColor::Yellow, false, -1.f, 0);
		}
	}

	const FString Text = FString::Printf(TEXT("[블랙홀 %s] 상태 %s | 남은 시간 %s | 영향 반경 %.0f | 포획 반경 %.0f | 끌리는 중 %d | 포획 %d"),
		HasAuthority() ? TEXT("서버") : TEXT("클라"),
		*UEnum::GetDisplayValueAsText(State).ToString(),
		*Remaining,
		GetInfluenceRadius(),
		GetInnerRadius(),
		PulledCount,
		CapturedCharacters.Num());
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Purple, Text);
#endif
}
