// Copyright (C) 2026 biksari studio. All Rights Reserved.


#include "Actors/Gimmick/Breakable/BreakableProp.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionSimulationTypes.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Utilities/TromboneLogs.h"

namespace
{
	/** 파편 프로파일 이름. PhysicsOnly 라서 어떤 씬 쿼리에도 잡히지 않는다 */
	const FName DebrisProfileName(TEXT("Debris"));
}

ABreakableProp::ABreakableProp()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	// 맵에 배치된 소품이라 부서지기 전까지 복제 비용이 0이다. Break()에서 FlushNetDormancy로 깨운다
	NetDormancy = DORM_Initial;

	IntactMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IntactMesh"));
	SetRootComponent(IntactMesh);
	IntactMesh->SetGenerateOverlapEvents(false);
	IntactMesh->CanCharacterStepUpOn = ECB_No;
	IntactMesh->SetReceivesDecals(false);
	// 콜리전 프로파일은 여기서 정하지 않는다. 술잔(뚫림+오버랩)과 문(벽처럼 막음)이 다르므로
	// 서브클래스 생성자가 각자 지정한다.

	Debris = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("Debris"));
	Debris->SetupAttachment(IntactMesh);
	Debris->SetVisibility(false);
	// GC 생성자 기본값이 true다. 프록시를 만들지 않아야 부서지기 전 비용이 0이 된다
	Debris->BodyInstance.bSimulatePhysics = false;
	Debris->SetCollisionProfileName(DebrisProfileName);
	// 쿼리를 실제로 끄는 경로는 이 배열뿐이다. 프로파일만 걸면 파편이 쿼리에 잡혀 플레이어가 올라선다
	Debris->SetPerLevelCollisionProfileNames({ DebrisProfileName });
	Debris->SetEnableReplication(false);
	Debris->SetCanEverAffectNavigation(false);
	Debris->bEnableDamageFromCollision = false;
	Debris->OneWayInteractionLevel = 1;
	Debris->InitialVelocityType = EInitialVelocityTypeEnum::Chaos_Initial_Velocity_User_Defined;

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	if (AkComponent)
	{
		AkComponent->OcclusionRefreshInterval = 0.f;
		AkComponent->SetupAttachment(IntactMesh);
	}
}

void ABreakableProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABreakableProp, bBroken);
	DOREPLIFETIME(ABreakableProp, LastHit);
}

void ABreakableProp::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// BP에서 비워 두면 파편이 쿼리에 잡힌다. 값을 읽을 수 없는 protected 배열이라 매번 덮어쓴다
	if (Debris)
	{
		Debris->SetPerLevelCollisionProfileNames({ DebrisProfileName });
	}
}

void ABreakableProp::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bBreakOnPawnTouch && IntactMesh)
	{
		IntactMesh->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnIntactMeshBeginOverlap);
	}
}

void ABreakableProp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(DebrisLifetimeHandle);
	Super::EndPlay(EndPlayReason);
}

bool ABreakableProp::Break_Implementation(const FBreakHitInfo& HitInfo)
{
	if (!HasAuthority() || bBroken) return false;

	if (HitInfo.Source == EBreakSource::PawnTouch && !bBreakOnPawnTouch) return false;
	if (HitInfo.Source == EBreakSource::Attack && !bBreakOnAttack) return false;

	// DORM_Initial 이면 DormantAll 로 바뀌면서 깨어난다. 이후 알아서 다시 잠든다
	FlushNetDormancy();

	bBroken = true;
	LastHit = HitInfo;

	// 리슨 서버는 자기 OnRep이 안 불리므로 직접 호출한다 (프로젝트 관용구)
	OnRep_Broken();
	ForceNetUpdate();

	OnBroken.Broadcast(this, LastHit);
	return true;
}

bool ABreakableProp::IsBroken_Implementation() const
{
	return bBroken;
}

void ABreakableProp::OnRep_Broken()
{
	if (bBroken)
	{
		PlayBreakLocal();
	}
}

void ABreakableProp::PlayBreakLocal()
{
	if (bBreakPlayed) return;
	bBreakPlayed = true;

	if (IntactMesh)
	{
		IntactMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		IntactMesh->SetGenerateOverlapEvents(false);
		// 전파하지 않는다 — 자식인 Debris 까지 숨어버린다
		IntactMesh->SetVisibility(false);
	}

	// 데디 서버는 연출을 돌리지 않는다. 위에서 콜리전만 끈 상태로 클라와 이동 판정이 같아진다
	if (GetNetMode() == NM_DedicatedServer) return;

	const FVector ImpactPoint = LastHit.ImpactPoint;
	const FVector ImpactDirection = FVector(LastHit.ImpactDirection).GetSafeNormal();

	if (Debris && Debris->GetRestCollection())
	{
		Debris->InitialVelocityType = EInitialVelocityTypeEnum::Chaos_Initial_Velocity_User_Defined;
		Debris->InitialLinearVelocity = ImpactDirection * LastHit.Strength * BreakSpeedScale;
		Debris->InitialAngularVelocity = FMath::VRand() * BreakSpinRadPerSec;

		Debris->SetVisibility(true);
		// 여기서 물리 프록시가 처음 만들어진다
		Debris->SetSimulatePhysics(true);
		// 루트 클러스터를 한 번에 조각으로 흩는다
		Debris->CrumbleActiveClusters();
		Debris->AddRadialImpulse(ImpactPoint, DebrisImpulseRadius, LastHit.Strength, RIF_Linear, /*bVelChange*/ true);

		GetWorldTimerManager().SetTimer(DebrisLifetimeHandle, this, &ThisClass::CleanupDebris, DebrisLifetimeSeconds, false);
	}

	if (BreakVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BreakVFX, ImpactPoint, ImpactDirection.Rotation());
	}

	if (AkComponent && BreakSoundEvent)
	{
		AkComponent->PostAkEvent(BreakSoundEvent, 0, FOnAkPostEventCallback());
	}
}

void ABreakableProp::CleanupDebris()
{
	if (Debris)
	{
		Debris->SetVisibility(false);
		Debris->DestroyComponent();
		Debris = nullptr;
	}
}

void ABreakableProp::OnIntactMeshBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || bBroken) return;

	APawn* TouchingPawn = Cast<APawn>(OtherActor);
	if (!TouchingPawn) return;

	FBreakHitInfo Info;
	Info.Source = EBreakSource::PawnTouch;
	Info.ImpactPoint = bFromSweep ? FVector(SweepResult.ImpactPoint) : TouchingPawn->GetActorLocation();

	FVector Direction = (GetActorLocation() - TouchingPawn->GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = TouchingPawn->GetActorForwardVector();
	}
	Info.ImpactDirection = Direction;
	Info.Strength = FMath::Max(MinTouchSpeed, static_cast<float>(TouchingPawn->GetVelocity().Size()));
	Info.Instigator = TouchingPawn;

	IBreakable::Execute_Break(this, Info);
}
