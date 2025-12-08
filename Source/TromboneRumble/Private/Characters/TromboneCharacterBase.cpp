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

void ATromboneCharacterBase::EnablePlayerInput()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			EnableInput(PlayerController);
		}
	}
	
	bIsCanProcessInput = true;
}

void ATromboneCharacterBase::DisablePlayerInput()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			DisableInput(PlayerController);
		}
	}
	
	bIsCanProcessInput = false;
}

void ATromboneCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* BaseSkinMat = GetMesh()->GetMaterial(1);
	SkinMID = GetMesh()->CreateDynamicMaterialInstance(1, BaseSkinMat);
	GetMesh()->SetMaterial(1, SkinMID);

	UMaterialInterface* BaseFaceMat = GetMesh()->GetMaterial(2);
	FaceMID = GetMesh()->CreateDynamicMaterialInstance(2, BaseFaceMat);
	GetMesh()->SetMaterial(2, FaceMID);

	SetupCharacterData();
	UpdateSkinFromPlayerState();

	PhysicalAnimationComp->SetSkeletalMeshComponent(GetMesh());
}

void ATromboneCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bIsRagdoll);
	DOREPLIFETIME(ThisClass, bIsStun);
	DOREPLIFETIME(ThisClass, SkinColor);
}

void ATromboneCharacterBase::OnHitReceived(const FHitData& HitData)
{
	if (!HasAuthority()) return;

	switch (HitData.HitType)
	{
		case EHitType::Headbutt:
			OnRagdoll();
			break;
		case EHitType::Instrument:
		case EHitType::Trombone:
		case EHitType::Cymbals:
		case EHitType::Violin:
			OnStun();
			break;
		case EHitType::Audience:
			break;
		default:
			break;
	}
}

void ATromboneCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// TODO : Remove debug drawing
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
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block); // Object Channel 1 : Weapon
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
}

void ATromboneCharacterBase::ApplyStun()
{
	StopAnimMontage();
	DisablePlayerInput();
}

void ATromboneCharacterBase::UnapplyStun()
{
	if (bIsRagdoll) return;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	EnablePlayerInput();
}

void ATromboneCharacterBase::ApplyRagdoll()
{
	DisablePlayerInput();
	
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

void ATromboneCharacterBase::InternalUnapplyRagdoll()
{
	UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInst) return;

	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -HalfHeight), FRotator(0.f, -90.f, 0.f));
	
	if (bRagdollOnGround)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		AnimInst->PlayGetUpMontage(IsFacingUp());
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		GetCharacterMovement()->Velocity = LastRagdollVelocity;
	}
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAllBodiesSimulatePhysics(false);
	AnimInst->SetIsRagdollBlending(false);
	AnimInst->SetIsRagdolling(false);
}

bool ATromboneCharacterBase::IsFacingUp() const
{
	if (!GetMesh()) return true;

	const FName PelvisSocketName = TEXT("pelvis"); 
	const FRotator PelvisRotation = GetMesh()->GetSocketRotation(PelvisSocketName);
	const FVector PelvisUp = FRotationMatrix(PelvisRotation).GetScaledAxis(EAxis::Z);
    
	return (FVector::DotProduct(PelvisUp, FVector::UpVector) > 0.0f);
}

void ATromboneCharacterBase::RagdollUpdate()
{
	LastRagdollVelocity = GetMesh()->GetPhysicsLinearVelocity(TEXT("root"));
	const float Spring = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 100.0f), FVector2D(1000.0f, 2800.0f), LastRagdollVelocity.Length());
	GetMesh()->SetAllMotorsAngularDriveParams(Spring, 0.0f, 0.0f, false);
	
	FPhysicalAnimationData StrengthData_0;
	StrengthData_0.bIsLocalSimulation = true;
	StrengthData_0.OrientationStrength = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 500.0f), FVector2D(0.0f, 1000.0f), LastRagdollVelocity.Length());
	
	FPhysicalAnimationData StrengthData_1;
	StrengthData_1.bIsLocalSimulation = true;
	StrengthData_1.OrientationStrength = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 500.0f), FVector2D(0.0f, 1000.0f), LastRagdollVelocity.Length());
	StrengthData_1.PositionStrength = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 500.0f), FVector2D(0.0f, 2000.0f), LastRagdollVelocity.Length());
	
	PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow("spine_03", StrengthData_0, true);
	PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow("thigh_l", StrengthData_0, true);
	PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow("thigh_r", StrengthData_0, true);
	PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow("hand_l", StrengthData_1, true);
	PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow("hand_r", StrengthData_1, true);
	
	GetMesh()->SetEnableGravity(LastRagdollVelocity.Z > -4000.0f);
	SetActorLocationDuringRagdoll();
}

void ATromboneCharacterBase::SetActorLocationDuringRagdoll()
{
	const FVector TargetRagdollLocation = GetMesh()->GetSocketLocation("pelvis");
	const FRotator PelvisRotation = GetMesh()->GetSocketRotation("pelvis");

	const FRotator TargetRagdollRotation = FRotator(0.0f, PelvisRotation.Yaw - (IsFacingUp() ? 180.0f : 0.0f), 0.0f);

	const float MeshHeightOffset = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector TraceEnd = TargetRagdollLocation - FVector(0.0f, 0.0f, MeshHeightOffset);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	bRagdollOnGround = GetWorld()->LineTraceSingleByChannel(HitResult, TargetRagdollLocation, TraceEnd, ECC_Visibility, QueryParams);
	if (bRagdollOnGround)
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

void ATromboneCharacterBase::OnRep_IsRagdoll()
{
	if (bIsRagdoll)
	{
		ApplyRagdoll();
		OnRagdollDelegate.Broadcast();
	}
	else
	{
		UnapplyRagdoll();
	}
}

void ATromboneCharacterBase::OnRep_IsStun()
{
	if (bIsStun)
	{
		ApplyStun();
		OnStunDelegate.Broadcast();
	}
	else
	{
		UnapplyStun();
	}
}
