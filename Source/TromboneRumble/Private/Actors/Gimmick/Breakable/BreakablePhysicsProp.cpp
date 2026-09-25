// Copyright (C) 2026 biksari studio. All Rights Reserved.


#include "Actors/Gimmick/Breakable/BreakablePhysicsProp.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Utilities/TromboneLogs.h"

ABreakablePhysicsProp::ABreakablePhysicsProp()
{
	// 닿으면 부서지는 대신 차인다. 공격에는 그대로 부서진다
	bBreakOnPawnTouch = false;
	bBreakOnAttack = true;

	// 서버가 위치 권한을 갖고, 클라도 시뮬하면서 보정받는다
	SetReplicatingMovement(true);

	if (IntactMesh)
	{
		IntactMesh->SetCollisionProfileName(TEXT("BreakablePhysicsProp"));
		IntactMesh->SetGenerateOverlapEvents(true);
		IntactMesh->SetNotifyRigidBodyCollision(true);
		IntactMesh->BodyInstance.bSimulatePhysics = true;
		// 배치된 채로 잠들어 있어야 레벨 시작 때 흔들리거나 부서지지 않는다
		IntactMesh->BodyInstance.bStartAwake = false;
		IntactMesh->BodyInstance.bGenerateWakeEvents = true;
	}
}

void ABreakablePhysicsProp::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 레벨 배치본이 Static으로 저장돼 있으면 시뮬이 안 된다. SetMobility가 재등록하며 물리 바디를 다시 만든다
	if (IntactMesh && IntactMesh->Mobility != EComponentMobility::Movable)
	{
		UE_LOG(LogGimmick, Log, TEXT("[Breakable] %s: IntactMesh Mobility를 Movable로 바꿉니다 (배치본이 Static으로 저장됨)"), *GetName());
		IntactMesh->SetMobility(EComponentMobility::Movable);
	}
}

void ABreakablePhysicsProp::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !IntactMesh) return;

	IntactMesh->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnPhysicsBeginOverlap);
	IntactMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnPhysicsHit);
	IntactMesh->OnComponentWake.AddDynamic(this, &ThisClass::OnPhysicsWake);
	IntactMesh->OnComponentSleep.AddDynamic(this, &ThisClass::OnPhysicsSleep);
}

bool ABreakablePhysicsProp::Break_Implementation(const FBreakHitInfo& HitInfo)
{
	if (!Super::Break_Implementation(HitInfo)) return false;

	// 부서진 뒤엔 위치를 보낼 필요가 없다. bBroken 이 복제된 뒤 잠든다
	SetReplicateMovement(false);
	SetNetDormancy(DORM_DormantAll);
	return true;
}

void ABreakablePhysicsProp::OnPhysicsBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bBroken || !IntactMesh || !IntactMesh->IsSimulatingPhysics()) return;

	APawn* KickingPawn = Cast<APawn>(OtherActor);
	if (!KickingPawn) return;

	const double Now = GetWorld()->GetTimeSeconds();
	if (LastKickTime >= 0.0 && Now - LastKickTime < KickCooldown) return;
	LastKickTime = Now;

	FVector Direction = (GetActorLocation() - KickingPawn->GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = KickingPawn->GetActorForwardVector();
	}

	const float PawnSpeed = KickingPawn->GetVelocity().Size2D();
	const float KickSpeed = FMath::Max(MinKickSpeed, PawnSpeed * KickSpeedScale);
	const FVector KickVelocity = Direction * KickSpeed + FVector::UpVector * KickUpSpeed;

	IntactMesh->AddImpulse(KickVelocity, NAME_None, /*bVelChange*/ true);
}

void ABreakablePhysicsProp::OnPhysicsHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (bBroken || !IntactMesh) return;

	const float Mass = IntactMesh->GetMass();
	if (Mass <= KINDA_SMALL_NUMBER) return;

	// 이번 충돌로 바뀐 속도. 질량과 무관하게 비교할 수 있다
	const float ImpactSpeed = NormalImpulse.Size() / Mass;
	if (ImpactSpeed < BreakImpactSpeed) return;

	FVector Direction = IntactMesh->GetPhysicsLinearVelocity().GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = -FVector(Hit.ImpactNormal);
	}

	FBreakHitInfo Info;
	Info.Source = EBreakSource::Impact;
	Info.ImpactPoint = Hit.ImpactPoint;
	Info.ImpactDirection = Direction;
	Info.Strength = ImpactSpeed;
	Info.Instigator = OtherActor;

	IBreakable::Execute_Break(this, Info);
}

void ABreakablePhysicsProp::OnPhysicsWake(UPrimitiveComponent* WakingComponent, FName BoneName)
{
	if (bBroken) return;
	SetNetDormancy(DORM_Awake);
}

void ABreakablePhysicsProp::OnPhysicsSleep(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
	if (bBroken) return;
	// 멈춘 위치가 클라에 도착한 뒤에 잠든다
	SetNetDormancy(DORM_DormantAll);
}
