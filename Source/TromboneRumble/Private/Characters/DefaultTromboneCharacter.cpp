// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/DefaultPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Components/ActorComponents/AttackComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponents/InteractorComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "Framework/LobbyGameMode.h"
#include "Items/InstrumentBase.h"
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
}

void ADefaultTromboneCharacter::Equip(AItemBase* ItemToEquip)
{
	if (AInstrumentBase* Instrument = Cast<AInstrumentBase>(ItemToEquip))
	{
		EquippedInstrument = Instrument;
		CurrentInteractionContext.bIsEquipped = true;
		UpdateAttackComponentState();
		OnInstrumentEquippedDelegate.Broadcast(this, Instrument);
	}
}

void ADefaultTromboneCharacter::Unequip()
{
	OnInstrumentUnequippedDelegate.Broadcast(this, EquippedInstrument);
	
	if (HasAuthority())
	{
		EquippedInstrument = nullptr;
	}
	
	CurrentInteractionContext.bIsEquipped = false;
	UpdateAttackComponentState();
}

void ADefaultTromboneCharacter::Jump()
{
	if (bIsSprinting && !CurrentInteractionContext.bIsEquipped)
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

void ADefaultTromboneCharacter::Interact()
{
	if (InteractorComponent) InteractorComponent->TryInteract(CurrentInteractionContext);
}

void ADefaultTromboneCharacter::Attack()
{
	if (AttackComponent && CurrentInteractionContext.bIsEquipped) AttackComponent->Attack();
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

	InteractorComponent->OnInteractableAvailable.RemoveDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
	InteractorComponent->OnInteractableAvailable.AddDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
	InteractorComponent->OnInteractSuccessDelegate.AddDynamic(this, &ThisClass::HandleInteractSuccess);
	OnRagdollDelegate.AddDynamic(this, &ThisClass::HandleOnRagdoll);

	if (AttackComponent) UpdateAttackComponentState();
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
	DOREPLIFETIME(ThisClass, EquippedInstrument);
}

void ADefaultTromboneCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	
	if (const ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		GM->SubscribeCharacterEvents(this);
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
			IEquipable::Execute_Equip(NewInstrument, this);
			Equip(NewInstrument);
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
	Equip(InteractedItem);
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
	Unequip();
}

void ADefaultTromboneCharacter::OnRep_EquippedInstrument()
{
	UpdateAttackComponentState();
}

void ADefaultTromboneCharacter::UpdateAttackComponentState()
{
	if (!AttackComponent) return;

	if (EquippedInstrument)
	{
		UPrimitiveComponent* Collision = EquippedInstrument->GetCapsuleComponent();
		UAttackDataAsset* Data = EquippedInstrument->GetAttackData();
       
		AttackComponent->SetCollisionComponent(Collision);
		AttackComponent->SetAttackData(Data);
	}
	else
	{
		AttackComponent->SetCollisionComponent(HeadbuttCapsuleComponent);
		AttackComponent->SetAttackData(HeadbuttAttackData);
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
