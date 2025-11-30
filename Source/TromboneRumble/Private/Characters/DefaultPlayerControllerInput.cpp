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

void ADefaultPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	OnPlayerStateChanged.Broadcast(PlayerState);
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
		if (RhythmAction)  EIC->BindAction(RhythmAction, ETriggerEvent::Started, this, &ThisClass::Handle_Rhythm, true);
		if (RhythmAction)  EIC->BindAction(RhythmAction, ETriggerEvent::Completed, this, &ThisClass::Handle_Rhythm, false);
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
	if (CanProcessInput()) CachedOwnerCharacter->Move(Value);
}

void ADefaultPlayerController::Handle_JumpPressed()
{
	if (CanProcessInput()) CachedOwnerCharacter->Jump();
}

void ADefaultPlayerController::Handle_JumpReleased()
{
	if (CanProcessInput()) CachedOwnerCharacter->StopJumping();
}

void ADefaultPlayerController::Handle_Interact()
{
	if (CanProcessInput()) CachedOwnerCharacter->TryInteract();
}

void ADefaultPlayerController::Handle_SprintPressed()
{
	if (CanProcessInput()) CachedOwnerCharacter->StartSprint();
}

void ADefaultPlayerController::Handle_SprintReleased()
{
	if (CanProcessInput()) CachedOwnerCharacter->StopSprint();
}

void ADefaultPlayerController::Handle_Attack()
{
	if (CanProcessInput()) CachedOwnerCharacter->Attack();
}

void ADefaultPlayerController::Handle_Rhythm(const bool bPressed)
{
	if (CanProcessInput()) CachedOwnerCharacter->Rhythm(bPressed);
}

bool ADefaultPlayerController::CanProcessInput()
{
	return CachedOwnerCharacter.IsValid() && !CachedOwnerCharacter->IsStun() && !CachedOwnerCharacter->IsRagdoll();
}
