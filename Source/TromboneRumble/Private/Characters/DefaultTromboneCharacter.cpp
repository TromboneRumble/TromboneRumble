// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/DefaultPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Components/ActorComponents/AttackComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponents/InteractorComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "Items/InstrumentBase.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"

ADefaultTromboneCharacter::ADefaultTromboneCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 640.f;
	CameraBoom->SetRelativeRotation(FRotator(-42.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor"));
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
}

void ADefaultTromboneCharacter::Jump()
{
	const AItemBase* Weapon = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Weapon);
	
	if (bIsSprinting && !Weapon)
	{
		AttackComponent->Attack();
	}
	else
	{
		Super::Jump();
	}
}

void ADefaultTromboneCharacter::Move(const struct FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ADefaultTromboneCharacter::TryInteract()
{
	if (InteractorComponent) InteractorComponent->TryInteract(CurrentInteractionContext);
}

void ADefaultTromboneCharacter::Attack()
{
	const AItemBase* Weapon = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Weapon);
	
	if (AttackComponent && Weapon) AttackComponent->Attack();
}

void ADefaultTromboneCharacter::StartSprint()
{
	if (bIsSprinting) return; 

	bIsSprinting = true;
	Server_SetIsSprinting(true);
}

void ADefaultTromboneCharacter::StopSprint()
{
	if (!bIsSprinting) return;

	bIsSprinting = false;
	Server_SetIsSprinting(false);
}

void ADefaultTromboneCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CachedCharacterController = Cast<ADefaultPlayerController>(GetController());
	GetCachedRhythmActor();

	InteractorComponent->OnInteractableAvailable.RemoveDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
	InteractorComponent->OnInteractableAvailable.AddDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
	InteractorComponent->OnInteractSuccessDelegate.AddDynamic(this, &ThisClass::HandleInteractSuccess);
	EquipmentComponent->OnEquipmentChangedDelegate.AddDynamic(this, &ThisClass::HandleOnEquipmentChanged);
	OnRagdollDelegate.AddDynamic(this, &ThisClass::HandleOnRagdoll);

	HandleOnEquipmentChanged(EEquipmentSlotType::Weapon, nullptr, nullptr);
	
}

void ADefaultTromboneCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	InterpolateMovementSpeed(DeltaSeconds);
}

void ADefaultTromboneCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bIsSprinting);
}

void ADefaultTromboneCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;

	if (EquipmentComponent)
	{
		EquipmentComponent->InitializeOwner(this);
	}
	
	const ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
	if (PS && PS->EquippedInstrumentClass)
	{
		UWorld* World = GetWorld();
		if (!World) return;
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		AInstrumentBase* NewInstrument = World->SpawnActor<AInstrumentBase>(PS->EquippedInstrumentClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (NewInstrument)
		{
			EquipmentComponent->TryEquipItem(NewInstrument);
		}
	}
}

void ADefaultTromboneCharacter::Server_SetIsSprinting_Implementation(const bool bNewIsSprinting)
{
	if (bIsSprinting != bNewIsSprinting)
	{
		bIsSprinting = bNewIsSprinting;
	}
}

void ADefaultTromboneCharacter::Server_InteractItem_Implementation(AItemBase* InteractedItem)
{
	EquipmentComponent->TryEquipItem(InteractedItem);
}

void ADefaultTromboneCharacter::HandleInteractableAvailableChanged(bool bAvailable)
{
	if (ADefaultPlayerController* PC = CachedCharacterController.Get())
	{
		PC->ShowInteractionUI(bAvailable);
	}
}

void ADefaultTromboneCharacter::HandleInteractSuccess(AActor* InteractedActor)
{
	if (!IsValid(InteractedActor)) return;

	if (AItemBase* Item = Cast<AItemBase>(InteractedActor))
	{
		Server_InteractItem(Item);
	}
}

void ADefaultTromboneCharacter::HandleOnRagdoll()
{
	EquipmentComponent->TryUnequipItem(EEquipmentSlotType::Weapon);
}

void ADefaultTromboneCharacter::HandleOnEquipmentChanged(const EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (!AttackComponent) return;
	
	if (Slot == EEquipmentSlotType::Weapon)
	{
		if (NewItem)
		{
			if (const AInstrumentBase* Instrument = Cast<AInstrumentBase>(NewItem))
			{
				AttackComponent->SetCollisionComponent(Instrument->GetCapsuleComponent());
				AttackComponent->SetAttackData(Instrument->GetAttackData());
				CurrentInteractionContext.bIsEquipped = true;
				if (GetCachedRhythmActor())
				{
					CachedRhythmActor->ExecuteOnInstrumentPicked(Instrument->GetInstrumentType());
				}
			}
		}
		else
		{
			AttackComponent->SetCollisionComponent(HeadbuttCapsuleComponent);
			AttackComponent->SetAttackData(HeadbuttAttackData);
			CurrentInteractionContext.bIsEquipped = false;
			if (GetCachedRhythmActor())
			{
				CachedRhythmActor->ExecuteOnInstrumentPicked(EInstrumentType::Background);
			}
		}
	}
}

void ADefaultTromboneCharacter::InterpolateMovementSpeed(const float DeltaSeconds) const
{
	const float TargetSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	const float CurrentSpeed = MoveComp->MaxWalkSpeed;
	if (!FMath::IsNearlyEqual(CurrentSpeed, TargetSpeed))
	{
		float NewSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaSeconds, SprintInterpSpeed);
		MoveComp->MaxWalkSpeed = NewSpeed;
	}
}

ARhythmActor* ADefaultTromboneCharacter::GetCachedRhythmActor()
{
	if (CachedRhythmActor.IsValid()) return CachedRhythmActor.Get();
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	if (ARhythmActor* FoundActor = Cast<ARhythmActor>(UGameplayStatics::GetActorOfClass(World, ARhythmActor::StaticClass())))
	{
		CachedRhythmActor = FoundActor;
		return CachedRhythmActor.Get();
	}
	return nullptr;
}
