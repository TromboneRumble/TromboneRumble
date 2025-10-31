// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/TromboneCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
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
	DOREPLIFETIME(ATromboneCharacterBase, bIsStun);
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
}

void ATromboneCharacterBase::InitCharacter()
{
	SetupCapsuleComponent();
	SetupSkeletalMeshComponent();
	SetupMovementComponent();
}

void ATromboneCharacterBase::SetupCapsuleComponent()
{
	HeadbuttCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HeadbuttCapsuleComponent"));
	HeadbuttCapsuleComponent->SetupAttachment(RootComponent);
	HeadbuttCapsuleComponent->SetCollisionObjectType(ECC_GameTraceChannel1) ; // Object Channel 1 : Weapon
	HeadbuttCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block); // Object Channel 1 : Weapon
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
		RagdollDuration, 
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
		StunDuration, 
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

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			DisableInput(PlayerController);
		}
	}
    
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
}

void ATromboneCharacterBase::UnapplyStun()
{
	if (bIsRagdoll) return;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			EnableInput(PlayerController);
		}
	}
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
