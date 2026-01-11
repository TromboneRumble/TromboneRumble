// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/TromboneCharacterBase.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Data/CharacterDataAsset.h"
#include "Framework/DefaultPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/Defines.h"

ATromboneCharacterBase::ATromboneCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PhysicalAnimationComp = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimationComponent"));
	
	InitCharacter();
}

void ATromboneCharacterBase::ApplySkinColor(const FLinearColor InSkinColor) const
{
	if (SkinMID)
	{
		SkinMID->SetVectorParameterValue(TEXT("BaseColor"), InSkinColor);
	}
	if (FaceMID)
	{
		FaceMID->SetVectorParameterValue(TEXT("BaseColor"), InSkinColor);
	}
}

void ATromboneCharacterBase::SetPlayerInput(const bool bShouldEnable)
{
	bIsCanProcessInput = bShouldEnable;

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			if (bShouldEnable)
			{
				EnableInput(PlayerController);
			}
			else
			{
				DisableInput(PlayerController);
			}
		}
	}
}

void ATromboneCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* BaseSkinMat = GetMesh()->GetMaterial(SkinMaterialIndex);
	SkinMID = GetMesh()->CreateDynamicMaterialInstance(SkinMaterialIndex, BaseSkinMat);
	GetMesh()->SetMaterial(SkinMaterialIndex, SkinMID);

	UMaterialInterface* BaseFaceMat = GetMesh()->GetMaterial(FaceMaterialIndex);
	FaceMID = GetMesh()->CreateDynamicMaterialInstance(FaceMaterialIndex, BaseFaceMat);
	GetMesh()->SetMaterial(FaceMaterialIndex, FaceMID);

	PlayFaceSequence(ECharacterFaceState::Blink);

	PhysicalAnimationComp->SetSkeletalMeshComponent(GetMesh());
	
	SetupCharacterData();
	UpdateSkinFromPlayerState();
	ApplyFlagPhysics();
}

void ATromboneCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bIsRagdoll);
	DOREPLIFETIME(ThisClass, bIsStun);
	DOREPLIFETIME(ThisClass, bIsInvincible);
	DOREPLIFETIME(ThisClass, SkinColor);
}

void ATromboneCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UpdateSkinFromPlayerState();
}

void ATromboneCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	UpdateSkinFromPlayerState();
}

void ATromboneCharacterBase::OnHitReceived_Implementation(const FHitData& HitData)
{
	if (!HasAuthority()) return;

	if (bIsInvincible || bIsStun || bIsRagdoll) return;

	switch (HitData.HitType)
	{
	case EHitReactionType::Ragdoll:
		OnRagdoll();
	case EHitReactionType::Stun:
		OnStun();
		break;
	case EHitReactionType::None:
	default:
		break;
	}

	LaunchCharacter(HitData.HitDirection * HitData.KnockbackForce, true, true);
}

void ATromboneCharacterBase::OnRep_SkinColor()
{
	ApplySkinColor(SkinColor);
}

void ATromboneCharacterBase::InitCharacter()
{
	SetupCapsuleComponent();
	SetupSkeletalMeshComponent();
}

void ATromboneCharacterBase::SetupCapsuleComponent()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap); // Object Channel 1 : Weapon
}

void ATromboneCharacterBase::SetupSkeletalMeshComponent()
{
	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -CapsuleHalfHeight), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->SetHiddenInGame(false);
}

void ATromboneCharacterBase::SetupCharacterData() const
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	
	if (CharacterData)
	{
		// Ground
		GetCharacterMovement()->MaxWalkSpeed = CharacterData->WalkSpeed;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, CharacterData->RotationRate, 0.0f);

		// Air
		GetCharacterMovement()->JumpZVelocity = CharacterData->JumpZVelocity;
		GetCharacterMovement()->AirControl = CharacterData->AirControl;

		// Inertia
		GetCharacterMovement()->GravityScale = CharacterData->GravityScale;
		GetCharacterMovement()->MaxAcceleration = CharacterData->MaxAcceleration;
		GetCharacterMovement()->BrakingDecelerationWalking = CharacterData->BrakingDecelerationWalking;
		GetCharacterMovement()->GroundFriction = CharacterData->GroundFriction;
	}
}

void ATromboneCharacterBase::OnRagdoll()
{
	if (!HasAuthority()) return;
	
	if (bIsRagdoll) return;

	if (bIsStun)
	{
		GetWorld()->GetTimerManager().ClearTimer(OnHitTimerHandle);
		bIsStun = false;
		OnRep_IsStun();
	}
	
	bIsRagdoll = true;
	OnRep_IsRagdoll();
	
	GetWorld()->GetTimerManager().SetTimer(
		OnHitTimerHandle, 
		this, 
		&ThisClass::EndRagdoll, 
		CharacterData->RagdollDuration, 
		false
	);
}

void ATromboneCharacterBase::EndRagdoll()
{
	if (!HasAuthority()) return;

	bIsRagdoll = false;
	OnRep_IsRagdoll();
	
	bIsInvincible = true;
	OnRep_IsInvincible();
	
	GetWorld()->GetTimerManager().SetTimer(
		InvincibilityTimerHandle, 
		[this]()
		{
			bIsInvincible = false;
			OnRep_IsInvincible();
		}, 
		CharacterData->InvincibilityDurationAfterRagdoll, 
		false
	);
}

void ATromboneCharacterBase::OnStun()
{
	if (!HasAuthority()) return;
    
	if (bIsRagdoll || bIsStun) return;

	bIsStun = true;
	OnRep_IsStun();

	GetWorld()->GetTimerManager().SetTimer(
		OnHitTimerHandle, 
		this, 
		&ThisClass::EndStun, 
		CharacterData->StunDuration, 
		false
	);
}

void ATromboneCharacterBase::EndStun()
{
	if (!HasAuthority()) return;

	bIsStun = false;
	OnRep_IsStun();
	
	bIsInvincible = true;
	OnRep_IsInvincible();
	
	GetWorld()->GetTimerManager().SetTimer(
		InvincibilityTimerHandle, 
		[this]()
		{
			bIsInvincible = false;
			OnRep_IsInvincible();
		}, 
		CharacterData->InvincibilityDurationAfterStun, 
		false
	);
}

void ATromboneCharacterBase::ApplyStun()
{
	StopAnimMontage();
	SetPlayerInput(false);
}

void ATromboneCharacterBase::UnapplyStun()
{
	if (bIsRagdoll) return;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	SetPlayerInput(true);
}

void ATromboneCharacterBase::ApplyRagdoll()
{
	SetPlayerInput(false);

    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
    
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->SetIsRagdolling(true);
	}
}

void ATromboneCharacterBase::UnapplyRagdoll()
{
    const FVector PelvisLocation = GetMesh()->GetSocketLocation(PelvisBoneName);
    const FRotator PelvisRotation = GetMesh()->GetSocketRotation(PelvisBoneName);

    FVector TargetCapsuleLocation = PelvisLocation;
    const FRotator TargetCapsuleRotation = FRotator(0.0f, PelvisRotation.Yaw + 90.0f, 0.0f);

    const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    FHitResult HitResult;
    FVector Start = PelvisLocation;
    FVector End = PelvisLocation - FVector(0.0f, 0.0f, CapsuleHalfHeight * 2.0f);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
       TargetCapsuleLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, CapsuleHalfHeight + 2.0f);
    }

    SetActorLocationAndRotation(TargetCapsuleLocation, TargetCapsuleRotation);
    GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0.0f, -90.0f, 0.0f));

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle, 
		this, 
		&ThisClass::DelayedSavePoseSnapshot, 
		PoseSnapshotInterval,
		false
	);
}

void ATromboneCharacterBase::DelayedSavePoseSnapshot()
{
	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->SaveRagdollPoseSnapshot();
	}

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle, 
		this, 
		&ThisClass::InternalUnapplyRagdoll, 
		PoseSnapshotInterval,
		false
	);
}
void ATromboneCharacterBase::InternalUnapplyRagdoll()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->PlayGetUpMontage(IsFacingUp());
	}

	ApplyFlagPhysics();
}

void ATromboneCharacterBase::UpdateSkinFromPlayerState()
{
	if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
	{
		SkinColor = DPS->GetSkinColor();
		ApplySkinColor(SkinColor);
	}
}

void ATromboneCharacterBase::UpdateFaceExpression(ECharacterFaceType NewType)
{
	if (FaceMID)
	{
		FaceMID->SetScalarParameterValue(FaceExpressionParameterName, static_cast<float>(NewType));
	}
}

void ATromboneCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
#if !UE_BUILD_SHIPPING
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &ATromboneCharacterBase::Server_DebugRagdoll);
	PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &ATromboneCharacterBase::Server_DebugStun);
#endif
}

void ATromboneCharacterBase::Server_DebugStun_Implementation()
{
	if (bIsStun)
	{
		GetWorld()->GetTimerManager().ClearTimer(OnHitTimerHandle);
		EndStun();
	}
	else
	{
		OnStun();
	}
}

void ATromboneCharacterBase::Server_DebugRagdoll_Implementation()
{
	if (bIsRagdoll)
	{
		GetWorld()->GetTimerManager().ClearTimer(OnHitTimerHandle);
		EndRagdoll();
	}
	else
	{
		OnRagdoll();
	}
}

void ATromboneCharacterBase::PlayFaceSequence(const ECharacterFaceState TargetState)
{
	if (!CharacterData) return;

	if (const FCharacterFaceAnimationSequence* FaceAnimData = CharacterData->FaceSequences.Find(TargetState))
	{
		InternalPlayFaceSequence(FaceAnimData);
	}
}

void ATromboneCharacterBase::InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence)
{
	GetWorld()->GetTimerManager().ClearTimer(FaceSequenceTimerHandle);
	CurrentActiveSequence = *InSequence;
	CurrentSequenceStep = 0;
	ExecuteFaceStep();
}

void ATromboneCharacterBase::ExecuteFaceStep()
{
	if (CurrentActiveSequence.Sequence.Num() == 0) return;

	UpdateFaceExpression(CurrentActiveSequence.Sequence[CurrentSequenceStep]);
	CurrentSequenceStep++;

	if (CurrentSequenceStep < CurrentActiveSequence.Sequence.Num())
	{
		GetWorld()->GetTimerManager().SetTimer(FaceSequenceTimerHandle, this, &ThisClass::ExecuteFaceStep, CurrentActiveSequence.Interval, false);
	}
	else if (CurrentActiveSequence.bLoop)
	{
		CurrentSequenceStep = 0;
		
		float NextDelay = FMath::FRandRange(CurrentActiveSequence.MinLoopDelay, CurrentActiveSequence.MaxLoopDelay);
		if (NextDelay <= 0.0f) NextDelay = CurrentActiveSequence.Interval;

		GetWorld()->GetTimerManager().SetTimer(FaceSequenceTimerHandle, this, &ThisClass::ExecuteFaceStep, NextDelay, false);
	}
}

bool ATromboneCharacterBase::IsFacingUp() const
{
	if (!GetMesh()) return true;

	const FRotator PelvisRotation = GetMesh()->GetSocketRotation(PelvisBoneName);
	const FVector PelvisUp = FRotationMatrix(PelvisRotation).GetScaledAxis(EAxis::Z);
    
	return (FVector::DotProduct(PelvisUp, FVector::UpVector) > 0.0f);
}

void ATromboneCharacterBase::ApplyFlagPhysics()
{
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;

	const UGameStateSubsystem* GameStateSubsystem = GI->GetSubsystem<UGameStateSubsystem>();
	if (!GameStateSubsystem || GameStateSubsystem->GetGameState() != EGameState::InGame) return;
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	FPhysicalAnimationData FlagAnimData;
	FlagAnimData.bIsLocalSimulation = false;
	FlagAnimData.OrientationStrength = 10.0f;
	FlagAnimData.AngularVelocityStrength = 5.0f;
	FlagAnimData.PositionStrength = 10.0f;
	FlagAnimData.VelocityStrength = 0.0f;
	FlagAnimData.MaxAngularForce = 0.0f;
	FlagAnimData.MaxLinearForce = 0.0f;

	FName BoneName = FName("flage01");
	GetMesh()->SetAllBodiesBelowSimulatePhysics(BoneName, true, true);
	PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow(BoneName, FlagAnimData, true);
}

void ATromboneCharacterBase::OnRep_IsRagdoll()
{
	if (bIsRagdoll)
	{
		ApplyRagdoll();
		PlayFaceSequence(ECharacterFaceState::Ragdoll);
		OnRagdollDelegate.Broadcast();
	}
	else
	{
		UnapplyRagdoll();
		PlayFaceSequence(ECharacterFaceState::Blink);
		EndRagdollDelegate.Broadcast();
	}
}

void ATromboneCharacterBase::OnRep_IsStun()
{
	if (bIsStun)
	{
		ApplyStun();
		PlayFaceSequence(ECharacterFaceState::Stun);
		OnStunDelegate.Broadcast();
	}
	else
	{
		UnapplyStun();
		PlayFaceSequence(ECharacterFaceState::Blink);
		EndStunDelegate.Broadcast();
	}
}

void ATromboneCharacterBase::OnRep_IsInvincible()
{
	if (bIsInvincible)
	{
		OnInvincibleDelegate.Broadcast();
	}
	else
	{
		EndInvincibleDelegate.Broadcast();
	}
}