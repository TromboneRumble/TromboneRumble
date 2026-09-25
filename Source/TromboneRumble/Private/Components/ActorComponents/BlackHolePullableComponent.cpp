// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/BlackHolePullableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/WorldSettings.h"
#include "TimerManager.h"
#include "Utilities/TromboneLogs.h"

UBlackHolePullableComponent::UBlackHolePullableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// 힘은 이 프레임의 물리 스텝에 들어가야 한다. 뒤 그룹에서 주면 한 프레임 늦게 반영된다
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UBlackHolePullableComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	UWorld* World = GetWorld();

	// 클라는 복제된 움직임만 받는다. 힘을 같이 주면 서버와 어긋난다
	if (!Owner || !World || !Owner->HasAuthority()) return;

	SpawnTransform = Owner->GetActorTransform();

	// 레벨에 블랙홀은 하나뿐이다 (AGimmickManager 가 타입당 1개만 등록한다)
	for (TActorIterator<ABlackHoleGimmick> It(World); It; ++It)
	{
		Gimmick = *It;
		break;
	}

	if (!Gimmick.IsValid())
	{
		UE_LOG(LogBlackHole, Warning, TEXT("[소품] %s: 레벨에 블랙홀 기믹이 없어 이 소품은 끌려가지 않습니다"), *Owner->GetName());
		return;
	}

	Gimmick->OnBlackHoleStateChangedDelegate.AddDynamic(this, &ThisClass::HandleBlackHoleStateChanged);
	Gimmick->OnBlackHoleBurstDelegate.AddDynamic(this, &ThisClass::HandleBlackHoleBurst);

	// 이미 돌고 있는 블랙홀에 늦게 들어온 소품도 바로 끌리게 현재 상태를 한 번 반영한다
	HandleBlackHoleStateChanged(Gimmick->GetState());

	World->GetTimerManager().SetTimer(ResetTimerHandle, this, &ThisClass::CheckFallReset,
		FMath::Max(0.1f, ResetCheckInterval), true);
}

void UBlackHolePullableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResetTimerHandle);
	}

	if (ABlackHoleGimmick* Hole = Gimmick.Get())
	{
		Hole->OnBlackHoleStateChangedDelegate.RemoveAll(this);
		Hole->OnBlackHoleBurstDelegate.RemoveAll(this);
	}

	// 댐핑과 중력을 되돌려 놓는다. 빠지면 소품이 영구히 떠 있거나 저항 없이 굴러다닌다
	StopPull();

	Super::EndPlay(EndPlayReason);
}

void UBlackHolePullableComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ABlackHoleGimmick* Hole = Gimmick.Get();
	if (!Hole || !Hole->IsPulling())
	{
		StopPull();
		return;
	}

	UPrimitiveComponent* Primitive = GetTargetPrimitive();
	if (!Primitive || !Primitive->IsAnySimulatingPhysics()) return;

	// 메시 피벗이 바닥에 있는 소품이 많아 무게중심으로 재야 고리 높이가 맞는다
	const FVector Location = Primitive->GetCenterOfMass();

	// 한 번 잡히면 밖으로 밀려나도 놓지 않는다. 방출까지 고리에 매달려 있어야 한다
	if (!bCaptured && Hole->IsInsideInner(Location))
	{
		Capture();
	}

	const FVector Velocity = Primitive->GetPhysicsLinearVelocity();
	const FVector Accel = bCaptured
		? Hole->SteerToward(Hole->ComputeRingVelocity(Location, RingSlot), Velocity)
		: Hole->ComputeSteerAccel(Location, Velocity, PullScale);

	if (Accel.IsNearlyZero()) return;

	// 천천히 끌리다 다시 잠든 바디는 힘을 무시한다. 끌림이 시작될 때 한 번 깨우는 것만으로는 부족하다
	if (!Primitive->RigidBodyIsAwake())
	{
		Primitive->WakeAllRigidBodies();
	}

	// 서브스테핑이 없어서 임펄스를 쌓으면 프레임레이트에 따라 결과가 달라진다. 가속도로 속도를 조향한다
	Primitive->AddForce(Accel, NAME_None, /*bAccelChange*/ true);
}

void UBlackHolePullableComponent::HandleBlackHoleStateChanged(const EBlackHoleState NewState)
{
	// 어느 상태가 끌어당기는지는 기믹만 알면 된다. 여기서 상태 목록을 다시 적지 않는다
	const ABlackHoleGimmick* Hole = Gimmick.Get();
	if (Hole && Hole->IsPulling())
	{
		StartPull();
		return;
	}

	StopPull();
}

void UBlackHolePullableComponent::HandleBlackHoleBurst(const FVector Center)
{
	// 고리에 매달려 있던 것만 튕긴다. 멀리서 끌리던 소품은 그 자리에 남는다
	if (!bCaptured) return;

	ABlackHoleGimmick* Hole = Gimmick.Get();
	UPrimitiveComponent* Primitive = GetTargetPrimitive();
	if (!Hole || !Primitive) return;

	// 중력과 댐핑은 바로 뒤에 오는 Idle 전이의 StopPull 이 되돌린다
	const FVector BurstVelocity = Hole->ComputeBurstVelocity(Primitive->GetCenterOfMass());
	Primitive->AddImpulse(BurstVelocity, NAME_None, /*bVelChange*/ true);

	UE_LOG(LogBlackHole, Log, TEXT("[소품] %s 방출 (%.0f cm/s)"), *GetOwner()->GetName(), BurstVelocity.Size());
}

void UBlackHolePullableComponent::StartPull()
{
	if (bPulling) return;
	bPulling = true;

	SetComponentTickEnabled(true);

	UPrimitiveComponent* Primitive = GetTargetPrimitive();
	if (!Primitive) return;

	// 조향이 이미 감쇠 역할을 한다. 물체의 댐핑까지 겹치면 원하는 속도에 한참 못 미친다
	SavedLinearDamping = Primitive->GetLinearDamping();
	SavedAngularDamping = Primitive->GetAngularDamping();

	// 포획에서 저장하면, 포획 없이 끝난 소품의 중력까지 강제로 켜 버린다
	bSavedGravityEnabled = Primitive->IsGravityEnabled();
	Primitive->SetLinearDamping(0.f);
	Primitive->SetAngularDamping(0.f);

	// 바닥에 놓인 소품은 잠들어 있어서 힘을 무시한다
	Primitive->WakeAllRigidBodies();
}

void UBlackHolePullableComponent::StopPull()
{
	if (!bPulling) return;
	bPulling = false;
	bCaptured = false;

	SetComponentTickEnabled(false);

	if (UPrimitiveComponent* Primitive = GetTargetPrimitive())
	{
		Primitive->SetLinearDamping(SavedLinearDamping);
		Primitive->SetAngularDamping(SavedAngularDamping);

		// 켜는 게 아니라 저작값으로 되돌린다. 우주정거장에는 중력을 끈 소품이 있을 수 있다
		Primitive->SetEnableGravity(bSavedGravityEnabled);
	}
}

void UBlackHolePullableComponent::Capture()
{
	ABlackHoleGimmick* Hole = Gimmick.Get();
	UPrimitiveComponent* Primitive = GetTargetPrimitive();
	if (!Hole || !Primitive) return;

	bCaptured = true;
	RingSlot = Hole->MakeRingSlot();

	// 중력이 남아 있으면 접선 운동이 계속 깎여서 고리가 안 만들어진다. 힘으로 상쇄하지 않고 아예 끈다
	Primitive->SetEnableGravity(false);

	UE_LOG(LogBlackHole, Log, TEXT("[소품] %s 포획 (반경 %+.0f, 높이 %+.0f, %.0f도/초)"),
		*GetOwner()->GetName(), RingSlot.RadiusOffset, RingSlot.HeightOffset, RingSlot.AngularSpeed);
}

void UBlackHolePullableComponent::CheckFallReset()
{
	AActor* Owner = GetOwner();
	if (!Owner || Owner->GetActorLocation().Z > GetResetZ()) return;

	// AResetCollider 는 캐릭터와 아이템만 되돌린다. 소품은 스스로 제자리로 간다
	if (UPrimitiveComponent* Primitive = GetTargetPrimitive())
	{
		Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	Owner->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);

	UE_LOG(LogBlackHole, Log, TEXT("[소품] %s 낙사 복귀"), *Owner->GetName());
}

float UBlackHolePullableComponent::GetResetZ() const
{
	if (!FMath::IsNearlyZero(ResetZ)) return ResetZ;

	// KillZ 보다 살짝 위. 엔진이 파괴하기 전에 되돌린다
	const AWorldSettings* Settings = GetWorld() ? GetWorld()->GetWorldSettings() : nullptr;
	return Settings ? Settings->KillZ + 200.f : -10000.f;
}

UPrimitiveComponent* UBlackHolePullableComponent::GetTargetPrimitive() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Cast<UPrimitiveComponent>(Owner->GetRootComponent()) : nullptr;
}
