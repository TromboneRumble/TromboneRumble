// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProtoType/TromboneRumbleCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "ProtoType/Trumpet.h"
#include "ProtoType/InGamePlayerController.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATromboneRumbleCharacter

ATromboneRumbleCharacter::ATromboneRumbleCharacter()
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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATromboneRumbleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
		CheckForInteraction();
}

void ATromboneRumbleCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATromboneRumbleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATromboneRumbleCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATromboneRumbleCharacter::Look);
		EnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Started, this, &ATromboneRumbleCharacter::Interaction);
		EnhancedInputComponent->BindAction(TackleAction, ETriggerEvent::Started, this, &ATromboneRumbleCharacter::Tackle);
	}
}

void ATromboneRumbleCharacter::Move(const FInputActionValue& Value)
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

void ATromboneRumbleCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATromboneRumbleCharacter::Interaction()
{
	if (!FocusedTrumpet) return;

	Server_Interaction(FocusedTrumpet);
}

void ATromboneRumbleCharacter::Tackle()
{
	if (HeldTrumpet) return;

	Server_Tackle();
}

void ATromboneRumbleCharacter::Drop()
{
	Server_Drop();
}

void ATromboneRumbleCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATromboneRumbleCharacter, bIsTackling);
}

void ATromboneRumbleCharacter::Server_Drop_Implementation()
{
	if (!HeldTrumpet) return;

	HeldTrumpet->OnGrab(false, this);
	HeldTrumpet = nullptr;
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
}

void ATromboneRumbleCharacter::Server_Tackle_Implementation()
{
	if (bIsTackling) return;

	bIsTackling = true;

    FTimerHandle TimerHandle_TackleEnd;
	GetWorldTimerManager().SetTimer(TimerHandle_TackleEnd, this, &ATromboneRumbleCharacter::EndTackleAnimation, TackleAnimationDuration, false);
	
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
	
	ATromboneRumbleCharacter* HitCharacter = Cast<ATromboneRumbleCharacter>(HitResult.GetActor());
	if (HitCharacter && HitCharacter->HeldTrumpet)
	{
		HitCharacter->Drop();
	}
}

void ATromboneRumbleCharacter::Server_Interaction_Implementation(ATrumpet* TrumpetToGrab)
{
	if (!TrumpetToGrab || !TrumpetToGrab->CanInteract()) return;

	TrumpetToGrab->OnGrab(true, this);
	HeldTrumpet = TrumpetToGrab;
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;
}

void ATromboneRumbleCharacter::CheckForInteraction()
{
	const AInGamePlayerController* PC = Cast<AInGamePlayerController>(GetController());
	if (!PC) return;
	
	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
	FVector WorldLocation, WorldDirection;
    
	if (!PC->DeprojectScreenPositionToWorld(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f, WorldLocation, WorldDirection)) return;

	const FVector Start = WorldLocation;
	const FVector End = Start + WorldDirection * InteractionDistance;
    
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
    
	FHitResult HitResult;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		if (ATrumpet* HitTrumpet = Cast<ATrumpet>(HitResult.GetActor()))
		{
			if (!HitTrumpet->CanInteract()) return;
			
			FocusedTrumpet = HitTrumpet;
			PC->ShowInteractionUI(true);
			return;
		}
	}
    
	if (FocusedTrumpet)
	{
		PC->ShowInteractionUI(false);
		FocusedTrumpet = nullptr;
	}
}

void ATromboneRumbleCharacter::EndTackleAnimation()
{
	if (HasAuthority())
	{
		bIsTackling = false;
	}
}