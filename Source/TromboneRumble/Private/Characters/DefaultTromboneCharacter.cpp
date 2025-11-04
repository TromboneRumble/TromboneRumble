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
#include "Items/InstrumentBase.h"

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
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
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

void ADefaultTromboneCharacter::Jump()
{
	if (bIsSprinting && !EquippedInstrument)
	{
		Attack();
	}
	else
	{
		Super::Jump();
	}
}

void ADefaultTromboneCharacter::StopJumping()
{
	Super::StopJumping();
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
	if (AttackComponent) AttackComponent->Attack();
}

void ADefaultTromboneCharacter::Sprint()
{
	if (bIsSprinting) return; 

	bIsSprinting = true;
	Server_StartSprint();
}

void ADefaultTromboneCharacter::StopSprint()
{
	if (!bIsSprinting) return;

	bIsSprinting = false;
	Server_StopSprint();
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
	
	const ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
	if (PS && PS->EquippedInstrumentClass)
	{
		UWorld* World = GetWorld();
		if (!World) return;
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		AInstrumentBase* NewInstrument = World->SpawnActor<AInstrumentBase>(
			PS->EquippedInstrumentClass, 
			GetActorLocation(),
			GetActorRotation(),
			SpawnParams
		);

		if (NewInstrument)
		{
			IEquipable::Execute_Equip(NewInstrument, this);
			EquippedInstrument = NewInstrument;
			CurrentInteractionContext.bIsEquipped = true;
			UpdateAttackComponentState();
		}
	}
}

void ADefaultTromboneCharacter::Server_StartSprint_Implementation()
{
	bIsSprinting = true;
}

void ADefaultTromboneCharacter::Server_StopSprint_Implementation()
{
	bIsSprinting = false;
}

void ADefaultTromboneCharacter::Server_Interact_Implementation(AActor* InteractedActor)
{
	if (const TObjectPtr<AInstrumentBase> Instrument = Cast<AInstrumentBase>(InteractedActor))
	{
		EquippedInstrument = Instrument;
		CurrentInteractionContext.bIsEquipped = true;
		UpdateAttackComponentState();
	}
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
	if (IsValid(InteractedActor) && InteractedActor->IsA<AInstrumentBase>())
	{
		Server_Interact(InteractedActor);
	}
}

void ADefaultTromboneCharacter::HandleOnRagdoll()
{
	if (HasAuthority())
	{
		EquippedInstrument = nullptr;
	}
	
	CurrentInteractionContext.bIsEquipped = false;
	UpdateAttackComponentState();
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
	if (auto* MovementComponent = GetCharacterMovement())
	{
		const float TargetSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;

		if (MovementComponent->MaxWalkSpeed != TargetSpeed)
		{
			MovementComponent->MaxWalkSpeed = FMath::FInterpTo(
				MovementComponent->MaxWalkSpeed,
				TargetSpeed,
				DeltaSeconds,
				SprintInterpSpeed
			);
		}
	}
}