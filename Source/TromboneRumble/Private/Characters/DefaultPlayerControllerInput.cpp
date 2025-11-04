// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultPlayerController.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Framework/LobbyGameMode.h"

void ADefaultPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);
	
	CachedOwnerCharacter = Cast<ADefaultTromboneCharacter>(InPawn);
}

void ADefaultPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)    EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Handle_Move);
		if (InteractAction)EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Handle_Interact);
		if (JumpAction)    EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Handle_JumpPressed);
		if (JumpAction)    EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::Handle_JumpReleased);
		if (SprintAction)  EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::Handle_SprintPressed);
		if (SprintAction)  EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::Handle_SprintReleased);
		if (AttackAction)  EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::Handle_Attack);
	}
}

void ADefaultPlayerController::HandleGameStateChanged(EGameState NewState)
{
	if (const ULocalPlayer* Lp = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = Lp->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();

			switch (NewState)
			{
			case EGameState::Lobby:
				if (LobbyMappingContext) Subsystem->AddMappingContext(LobbyMappingContext, 0);
				break;
			case EGameState::InGame:
				if (InGameMappingContext) Subsystem->AddMappingContext(InGameMappingContext, 0);
				break;
			default:
				break;
			}
		}
	}
}

void ADefaultPlayerController::Handle_Move(const struct FInputActionValue& Value)
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Move(Value);
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

void ADefaultPlayerController::Handle_Attack()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Attack();
}
