// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/Garbage/GarbageBase.h"

#include "AkComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"

AGarbageBase::AGarbageBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);

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

		MeshComp->SetLinearDamping(0.0f);
		MeshComp->SetAngularDamping(0.0f);
		MeshComp->SetNotifyRigidBodyCollision(true);
		MeshComp->SetGenerateOverlapEvents(false);
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetEnableGravity(true);
		MeshComp->CanCharacterStepUpOn = ECB_No;

		MeshComp->BodyInstance.bUseCCD = true;
	}

	TrailComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComp"));
	if (TrailComp)
	{
		TrailComp->SetupAttachment(RootComponent);
		TrailComp->SetAutoActivate(true);
	}

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	if (AkComponent)
	{
		AkComponent->SetupAttachment(RootComponent);
		AkComponent->OcclusionRefreshInterval = 0.f;
	}
}

void AGarbageBase::InitThrow_Server(const FVector& InStart, const FVector& InTarget)
{
	if (!HasAuthority() || !MeshComp)
	{
		return;
	}

	StartLoc = InStart;
	TargetLoc = InTarget;

	ChosenExtraApexHeight = FMath::FRandRange(MinExtraApexHeight, MaxExtraApexHeight);

	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetEnableGravity(true);

	const FVector V0 = ComputeBallisticInitialVelocity(InStart, InTarget, ChosenExtraApexHeight);

	MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	MeshComp->SetPhysicsLinearVelocity(V0, false);

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
	if (AkComponent && SpawnSoundEvent)
	{
		SpawnMusicPlayingID = AkComponent->PostAkEvent(SpawnSoundEvent, 0, FOnAkPostEventCallback());
	}
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
	DOREPLIFETIME(AGarbageBase, SpinRateDegPerSec);
	DOREPLIFETIME(AGarbageBase, bHitPawn);
}

void AGarbageBase::OnRep_ImpactStarted()
{
	if (bImpactStarted)
	{
		if (TrailComp)
		{
			TrailComp->Deactivate();
		}

		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice && SpawnMusicPlayingID != 0)
		{
			AudioDevice->StopPlayingID(SpawnMusicPlayingID);
			SpawnMusicPlayingID = 0;
		}
	}
}

void AGarbageBase::OnRep_HitPawn()
{
	if (bHitPawn)
	{
		if (AkComponent && HitSoundEvent)
		{
			AkComponent->PostAkEvent(HitSoundEvent, 0, FOnAkPostEventCallback());
		}
	}
}

void AGarbageBase::HandleMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                 FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || bImpactStarted)
	{
		return;
	}

	if (!IsValid(OtherActor) || OtherActor == this)
	{
		return;
	}

	if(OtherActor->Implements<UCombatReceiver>())
	{
		FHitData HitData;

		FVector Direction = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

		HitData.HitDirection = Direction;
		HitData.HitReaction = HitReactionType;
		HitData.KnockbackForce = KnockbackForce;
		HitData.KnockbackUpForce = KnockbackUpForce;
		HitData.HitInstigator = HitInstigatorType;

		ICombatReceiver::Execute_OnHitReceived(OtherActor, HitData);
	}

	const ECollisionChannel OtherObjType = OtherComp ? OtherComp->GetCollisionObjectType() : ECC_Visibility;
	const bool bHitWorld = (OtherObjType == ECC_WorldStatic) || (OtherObjType == ECC_WorldDynamic);
	const bool bOtherIsGarbage = OtherActor->IsA<AGarbageBase>();

	if (OtherActor->IsA<ACharacter>())
	{
		bHitPawn = true;
		OnRep_HitPawn();
		StartDestroyTimer_Server(DestroyDelayAfterImpact);
	}
	else if (bHitWorld && !bOtherIsGarbage)
	{
		StartDestroyTimer_Server(DestroyDelayAfterLand);
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

void AGarbageBase::StartDestroyTimer_Server(const float Delay)
{
	if (!HasAuthority() || bDestroyTimerStarted)
	{
		return;
	}

	bDestroyTimerStarted = true;
	bImpactStarted = true;
	OnRep_ImpactStarted();
	SetLifeSpan(Delay);
}