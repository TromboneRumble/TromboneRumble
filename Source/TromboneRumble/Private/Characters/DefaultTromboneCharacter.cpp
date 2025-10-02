// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/DefaultCharacterController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponents/InteractorComponent.h"


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
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor"));
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

void ADefaultTromboneCharacter::Look(const struct FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ADefaultTromboneCharacter::Interact()
{
	if (InteractorComponent)
	{
		InteractorComponent->TryInteract();
	}
}

void ADefaultTromboneCharacter::Tackle()
{
	if (!IsTackling())
	{
		Server_Tackle();
	}
}

void ADefaultTromboneCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADefaultTromboneCharacter, bIsTackling);
	DOREPLIFETIME(ADefaultTromboneCharacter, bHasTempTrumpet);
}

void ADefaultTromboneCharacter::EndTackleAnimation()
{
	if (HasAuthority())
	{
		bIsTackling = false;
	}
}

void ADefaultTromboneCharacter::Server_Interaction_Implementation(AActor* Interactable)
{
}

void ADefaultTromboneCharacter::Server_Tackle_Implementation()
{
	if (bIsTackling) return;

	bIsTackling = true;

	FTimerHandle TimerHandle_TackleEnd;
	GetWorldTimerManager().SetTimer(TimerHandle_TackleEnd, this, &ThisClass::EndTackleAnimation, TackleAnimationDuration, false);

	const FVector Start = GetActorLocation();
	const FVector End = Start + (GetActorForwardVector() * 200.0f);

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		Start,
		End,
		50.0f,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
	);

	if (!bHit) return;
	// TODO(next): if (HitActor implements UHitReactInterface) { IHitReactInterface::Execute_OnTackled(HitActor, this); }
	//ADefaultTromboneCharacter* HitCharacter = Cast<ADefaultTromboneCharacter>(HitResult.GetActor());
	//if (HitCharacter && HitCharacter->HeldTrumpet)
	//{
	//	HitCharacter->Drop();
	//}
	
}
