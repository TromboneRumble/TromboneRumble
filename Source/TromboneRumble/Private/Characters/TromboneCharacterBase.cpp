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

	const float FirstBlinkDelay = FMath::FRandRange(EyeBlinkingIntervalMin, EyeBlinkingIntervalMax);
	GetWorld()->GetTimerManager().SetTimer(BlinkingTimerHandle, this, &ATromboneCharacterBase::StartBlinking, FirstBlinkDelay, false);

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

void ATromboneCharacterBase::OnHitReceived(const FHitData& HitData)
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

void ATromboneCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// TODO : Remove debug drawing
	if (bIsInvincible)
	{
		DrawDebugString(
			GetWorld(),
			GetActorLocation() + FVector(0, 0, 150.0f),
			TEXT("무적"),
			nullptr,
			FColor::Red,
			0.0f,
			true
		);
	}
	if (bIsStun)
	{
		DrawDebugString(
			GetWorld(),
			GetActorLocation() + FVector(0, 0, 100.0f),
			TEXT("STUNNED"),
			nullptr,
			FColor::Red,
			0.0f,
			true
		);
	}

	if (bIsRagdoll)
	{
		RagdollUpdate();
	}
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
	StartBlinking();
}

void ATromboneCharacterBase::ApplyRagdoll()
{
	SetPlayerInput(false);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInst) return;
	AnimInst->SetIsRagdolling(true);
}

void ATromboneCharacterBase::UnapplyRagdoll()
{
	GetMesh()->SetSimulatePhysics(false);

	UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInst) return;
	
	const FName SnapshotName = TEXT("RagdollSnapshot");
	AnimInst->SavePoseSnapshot(SnapshotName);
	AnimInst->SetRagdollSnapshotName(SnapshotName);
	AnimInst->SetIsRagdollBlending(true);

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle, 
		this, 
		&ThisClass::InternalUnapplyRagdoll, 
		0.2f,
		false
	);
}

void ATromboneCharacterBase::UpdateSkinFromPlayerState()
{
	if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
	{
		SkinColor = DPS->GetSkinColor();
		ApplySkinColor(SkinColor);
	}
}

void ATromboneCharacterBase::UpdateFaceExpression(EFaceExpressionType NewType)
{
	if (FaceMID)
	{
		if (NewType == EFaceExpressionType::Stun || NewType == EFaceExpressionType::Ragdoll)
		{
			GetWorld()->GetTimerManager().ClearTimer(BlinkStepTimerHandle);
		}

		FaceMID->SetScalarParameterValue(FaceExpressionParameterName, static_cast<float>(NewType));
		CurrentExpressionType = NewType;
	}
}

void ATromboneCharacterBase::StartBlinking()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(BlinkStepTimerHandle)) return;
	
	if (!bIsStun && !bIsRagdoll)
	{
		BlinkStep = 0;
		ExecuteBlinkStep();
	}
	else
	{
		const float NextBlinkDelay = FMath::FRandRange(EyeBlinkingIntervalMin, EyeBlinkingIntervalMax);
		GetWorld()->GetTimerManager().SetTimer(BlinkingTimerHandle, this, &ATromboneCharacterBase::StartBlinking, NextBlinkDelay, false);
	}
}

void ATromboneCharacterBase::ExecuteBlinkStep()
{
	TArray BlinkSequence = { 0, 1, 2, 1, 0 };

	if (BlinkStep < BlinkSequence.Num())
	{
		UpdateFaceExpression(static_cast<EFaceExpressionType>(BlinkSequence[BlinkStep]));
		BlinkStep++;

		GetWorld()->GetTimerManager().SetTimer(BlinkStepTimerHandle, this, &ATromboneCharacterBase::ExecuteBlinkStep, 0.07f, false);
	}
	else
	{
		const float NextBlinkDelay = FMath::FRandRange(EyeBlinkingIntervalMin, EyeBlinkingIntervalMax);
		GetWorld()->GetTimerManager().SetTimer(BlinkingTimerHandle, this, &ATromboneCharacterBase::StartBlinking, NextBlinkDelay, false);
	}
}

void ATromboneCharacterBase::InternalUnapplyRagdoll()
{
	UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInst) return;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	AnimInst->PlayGetUpMontage(IsFacingUp());
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AnimInst->SetIsRagdollBlending(false);
	AnimInst->SetIsRagdolling(false);

	ApplyFlagPhysics();
	StartBlinking();
}

bool ATromboneCharacterBase::IsFacingUp() const
{
	if (!GetMesh()) return true;

	const FRotator PelvisRotation = GetMesh()->GetSocketRotation(PelvisBoneName);
	const FVector PelvisUp = FRotationMatrix(PelvisRotation).GetScaledAxis(EAxis::Z);
    
	return (FVector::DotProduct(PelvisUp, FVector::UpVector) > 0.0f);
}

void ATromboneCharacterBase::RagdollUpdate()
{
	const FVector LastRagdollVelocity = GetMesh()->GetPhysicsLinearVelocity(TEXT("root"));
	GetMesh()->SetEnableGravity(LastRagdollVelocity.Z > -4000.0f);
	SetActorLocationAndRotationDuringRagdoll();
}

void ATromboneCharacterBase::SetActorLocationAndRotationDuringRagdoll()
{
	const FVector TargetRagdollLocation = GetMesh()->GetSocketLocation(PelvisBoneName);
	const FRotator PelvisRotation = GetMesh()->GetSocketRotation(PelvisBoneName);

	const FRotator TargetRagdollRotation = FRotator(0.0f, PelvisRotation.Yaw + 90.0f, 0.0f);

	const float MeshHeightOffset = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector TraceEnd = TargetRagdollLocation - FVector(0.0f, 0.0f, MeshHeightOffset);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TargetRagdollLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		const float Offset = MeshHeightOffset - abs(HitResult.ImpactPoint.Z - HitResult.TraceStart.Z) + 2.0f;
		SetActorLocation(TargetRagdollLocation + FVector(0.0f, 0.0f, Offset));
	}
	else
	{
		SetActorLocation(TargetRagdollLocation);
	}
	
	SetActorRotation(TargetRagdollRotation);
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
		UpdateFaceExpression(EFaceExpressionType::Ragdoll);
		OnRagdollDelegate.Broadcast();
	}
	else
	{
		UnapplyRagdoll();
		EndRagdollDelegate.Broadcast();
	}
}

void ATromboneCharacterBase::OnRep_IsStun()
{
	if (bIsStun)
	{
		ApplyStun();
		UpdateFaceExpression(EFaceExpressionType::Stun);
		OnStunDelegate.Broadcast();
	}
	else
	{
		UnapplyStun();
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
