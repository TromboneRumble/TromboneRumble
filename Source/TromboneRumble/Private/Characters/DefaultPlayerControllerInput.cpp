// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultPlayerController.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Data/QuestData.h"
#include "Framework/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"

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
		if (GuideAction)   EIC->BindAction(GuideAction, ETriggerEvent::Started, this, &ThisClass::Handle_Guide);
		
		ATutorialManager* TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
		if (TutorialManager)
		{
			if (MoveAction) EIC->BindAction(MoveAction, ETriggerEvent::Started, TutorialManager, &ATutorialManager::ReportAction, EQuestConditionType::BasicAction, EQuestConditionParamType::Specific, FString("Move"));
			if (SprintAction) EIC->BindAction(SprintAction, ETriggerEvent::Started, TutorialManager, &ATutorialManager::ReportAction, EQuestConditionType::BasicAction, EQuestConditionParamType::Specific, FString("Run"));
			if (JumpAction) EIC->BindAction(JumpAction, ETriggerEvent::Started, TutorialManager, &ATutorialManager::ReportAction, EQuestConditionType::BasicAction, EQuestConditionParamType::Specific, FString("Jump"));
		}
	}
}

void ADefaultPlayerController::HandleLevelStateChanged(ELevelState NewState)
{
	if (const ULocalPlayer* Lp = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = Lp->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();

			switch (NewState)
			{
			case ELevelState::Lobby:
				if (LobbyMappingContext) Subsystem->AddMappingContext(LobbyMappingContext, 0);
				break;
			case ELevelState::InGame:
				if (InGameMappingContext) Subsystem->AddMappingContext(InGameMappingContext, 0);
				break;
			case ELevelState::Tutorial:
				if (InGameMappingContext) Subsystem->AddMappingContext(InGameMappingContext, 0);
				break;
			default:
				UE_LOG(LogTemp, Warning, TEXT("Unhandled level state."));
				break;
			}
		}
	}
}

void ADefaultPlayerController::HandleInGameStateChanged(const EInGameState NewState)
{
	switch (NewState)
	{
		case EInGameState::End:
		{
			DisableInput(this);
				
			if (const ULocalPlayer* LP = GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					Subsystem->RemoveMappingContext(InGameMappingContext);
				}
			}
    
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
			break;
		}
		default: 
			break;
	}
}

void ADefaultPlayerController::HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState)
{
	if (RhythmGameState == ERhythmGameState::Ended)
	{
		Server_RhythmGameFinished();
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

void ADefaultPlayerController::Handle_Guide()
{
	if (CanProcessInput()) CachedOwnerCharacter->ToggleGuideUI();
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
	return CachedOwnerCharacter.IsValid() && !CachedOwnerCharacter->IsStun() && !CachedOwnerCharacter->IsRagdoll() && CachedOwnerCharacter->IsCanProcessInput();
}
