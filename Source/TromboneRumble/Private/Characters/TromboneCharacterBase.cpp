// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/TromboneCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

ATromboneCharacterBase::ATromboneCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	InitCharacter();
}

void ATromboneCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ATromboneCharacterBase, bIsRagdoll);
}

void ATromboneCharacterBase::OnHitReceived(const FHitData& HitData)
{
	if (!HasAuthority()) return;

	if (HitData.HitType == EHitType::Headbutt)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Hit by Headbutt"));
	}
	else if (HitData.HitType == EHitType::Instrument)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Hit by Instrument"));
	}
	else
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Hit by Default"));
	}
	
	bIsRagdoll = true;

	OnRep_IsRagdoll();

	GetWorld()->GetTimerManager().SetTimer(RagdollTimerHandle, [this]()
		{
			if (HasAuthority())
			{
				bIsRagdoll = false;
				OnRep_IsRagdoll();
			}
		}, 
		RagdollDuration, false);
}

void ATromboneCharacterBase::InitCharacter() const
{
	SetupCapsuleComponent();
	SetupSkeletalMeshComponent();
	SetupMovementComponent();
}

void ATromboneCharacterBase::SetupCapsuleComponent() const
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ATromboneCharacterBase::SetupSkeletalMeshComponent() const
{
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->SetHiddenInGame(false);
}

void ATromboneCharacterBase::SetupMovementComponent() const
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
}

void ATromboneCharacterBase::ApplyRagdoll()
{
	if (GetMesh()->IsSimulatingPhysics()) return;
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			DisableInput(PlayerController);
		}
	}
	
	const FVector LastVelocity = GetCharacterMovement()->Velocity;

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->AddImpulse(LastVelocity, NAME_None, true);
}

void ATromboneCharacterBase::UnapplyRagdoll()
{
	if (!GetMesh()->IsSimulatingPhysics()) return;

	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			EnableInput(PlayerController);
		}
	}
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
