// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Garbage/GarbageBase.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

AGarbageBase::AGarbageBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	if (MeshComp)
	{
		SetRootComponent(MeshComp);

		MeshComp->bReceivesDecals = false;

		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
		MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

		//물리 이벤트로 판정
		MeshComp->SetNotifyRigidBodyCollision(true);
		MeshComp->SetGenerateOverlapEvents(false);
		//물리는 서버 권위로만 킴
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetEnableGravity(true);

		// 빠른 속도 관통 방지
		MeshComp->BodyInstance.bUseCCD = true;
	}

	TrailComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComp"));
	if (TrailComp)
	{
		TrailComp->SetupAttachment(RootComponent);
		TrailComp->SetAutoActivate(true);
	}
}

void AGarbageBase::InitThrow_Server(const FVector& InStart, const FVector& InTarget)
{
	if (!HasAuthority())
	{
		return;
	}

	StartLoc = InStart;
	TargetLoc = InTarget;

	ChosenExtraApexHeight = FMath::FRandRange(MinExtraApexHeight, MaxExtraApexHeight);

	if (!MeshComp)
	{
		return;
	}

	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetEnableGravity(true);

	const FVector V0 = ComputeBallisticInitialVelocity(InStart, InTarget, ChosenExtraApexHeight);

	// 기존 속도 제거 후 초기 속도 적용
	MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	MeshComp->SetPhysicsLinearVelocity(V0, false);

	// 랜덤 각속도 부여
	const float SpinX = FMath::FRandRange(MinSpinDegPerSec, MaxSpinDegPerSec) * (FMath::RandBool() ? 1.f : -1.f);
	const float SpinY = FMath::FRandRange(MinSpinDegPerSec, MaxSpinDegPerSec) * (FMath::RandBool() ? 1.f : -1.f);
	const float SpinZ = FMath::FRandRange(MinSpinDegPerSec, MaxSpinDegPerSec) * (FMath::RandBool() ? 1.f : -1.f);

	MeshComp->SetPhysicsAngularVelocityInDegrees(FVector(SpinX, SpinY, SpinZ), false);
}

void AGarbageBase::BeginPlay()
{
	Super::BeginPlay();
	if (MeshComp)
	{
		MeshComp->OnComponentHit.AddDynamic(this, &ThisClass::HandleMeshHit);
	}
	// 물리는 서버 권위로만 킴
	if (HasAuthority() && MeshComp)
	{
		MeshComp->SetSimulatePhysics(true);
	}
}

void AGarbageBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGarbageBase, StartLoc);
	DOREPLIFETIME(AGarbageBase, TargetLoc);
	DOREPLIFETIME(AGarbageBase, ChosenExtraApexHeight);
	DOREPLIFETIME(AGarbageBase, bImpactStarted);
}



void AGarbageBase::OnRep_ImpactStarted()
{
	if (bImpactStarted)
	{
		if (TrailComp)
		{
			TrailComp->Deactivate();
		}
	}
}

void AGarbageBase::HandleMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                 FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bImpactStarted)
	{
		return;
	}

	if (!IsValid(OtherActor) || OtherActor == this)
	{
		return;
	}

	//TODO : 플레이어 래그돌 로직 삽입
	if (OtherActor->IsA<ACharacter>())
	{
		UE_LOG(LogTemp, Warning, TEXT("Collided Actor : %s"), *OtherActor->GetName());
	}


	// 땅 또는 플레이어에 부딪힌 시점부터 삭제 타이머 시작
	// Garbage끼리 부딪히는 건 상관 없고 타이머 조건에는 포함하지 않는다
	const bool bHitPawn = OtherActor->IsA<ACharacter>();

	const ECollisionChannel OtherObjType = OtherComp ? OtherComp->GetCollisionObjectType() : ECC_Visibility;
	const bool bHitWorld = (OtherObjType == ECC_WorldStatic) || (OtherObjType == ECC_WorldDynamic);

	// WorldDynamic에는 다른 Garbage도 포함될 수 있다
	// Garbage끼리 충돌은 타이머 조건에서 제외한다
	const bool bOtherIsGarbage = OtherActor->IsA<AGarbageBase>();

	if (bHitPawn)
	{
		StartDestroyTimer_Server();
		return;
	}

	if (bHitWorld && !bOtherIsGarbage)
	{
		StartDestroyTimer_Server();
		return;
	}
}

FVector AGarbageBase::ComputeBallisticInitialVelocity(const FVector& InStart, const FVector& InTarget,
	float ExtraApexHeight) const
{
	const float GravityZ = GetWorld() ? -GetWorld()->GetGravityZ() : 980.f;
	const float Z0 = InStart.Z;
	const float Z1 = InTarget.Z;

	const float ZApex = FMath::Max(Z0, Z1) + FMath::Max(10.f, ExtraApexHeight);

	const float V0Z = FMath::Sqrt(2.f * GravityZ * FMath::Max(0.f, ZApex - Z0));
	const float TUp = V0Z / GravityZ;

	const float TDown = FMath::Sqrt(2.f * FMath::Max(0.f, ZApex - Z1) / GravityZ);
	const float TotalT = FMath::Max(0.05f, TUp + TDown);

	const FVector Delta = (InTarget - InStart);
	const FVector VelXY = FVector(Delta.X, Delta.Y, 0.f) / TotalT;

	return VelXY + FVector(0.f, 0.f, V0Z);
}

void AGarbageBase::StartDestroyTimer_Server()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bDestroyTimerStarted)
	{
		return;
	}

	bDestroyTimerStarted = true;
	bImpactStarted = true;

	if (TrailComp)
	{
		TrailComp->Deactivate();
	}

	// 충돌 이후 물리는 그대로 둔다
	// 굴러다니다가 시간 지나면 자동 삭제
	SetLifeSpan(DestroyDelayAfterImpact);
}
