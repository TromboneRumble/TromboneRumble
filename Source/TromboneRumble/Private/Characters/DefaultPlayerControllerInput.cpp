// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultPlayerController.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

void ADefaultPlayerController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);
	CachedOwnerCharacter = Cast<ADefaultTromboneCharacter>(APawn);
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext,0);
			}
		}
	}
}

void ADefaultPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	CachedOwnerCharacter = nullptr;
}

void ADefaultPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);
	CachedOwnerCharacter = Cast<ADefaultTromboneCharacter>(InPawn);
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ADefaultPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)    EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Handle_Move);
		//if (LookAction)    EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Handle_Look);
		if (InteractAction)EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Handle_Interact);
		if (JumpAction)    EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Handle_JumpPressed);
		if (JumpAction)    EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::Handle_JumpReleased);
		if (SprintAction)  EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::Handle_SprintPressed);
		if (SprintAction)  EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::Handle_SprintReleased);
	}
}

void ADefaultPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ADefaultPlayerController::Handle_Move(const struct FInputActionValue& Value)
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Move(Value);
}

void ADefaultPlayerController::Handle_Look(const struct FInputActionValue& Value)
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Look(Value);
}

void ADefaultPlayerController::Handle_JumpPressed()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Jump();
}

void ADefaultPlayerController::Handle_JumpReleased()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->StopJumping();
}

void ADefaultPlayerController::Handle_Interact()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Interact();
}

void ADefaultPlayerController::Handle_SprintPressed()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Sprint();
}

void ADefaultPlayerController::Handle_SprintReleased()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->StopSprint();
}