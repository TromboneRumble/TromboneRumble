#include "Characters/DefaultPlayerController.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Data/QuestData.h"
#include "Framework/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/VoiceChatSubsystem.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"
#include "Utilities/TromboneStatics.h"

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
		if (EscapeAction)  EIC->BindAction(EscapeAction, ETriggerEvent::Started, this, &ThisClass::Handle_Escape);
		if (CameraZoomAction) EIC->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &ThisClass::Handle_CameraZoom);
		if (PushToTalkAction)
		{
			EIC->BindAction(PushToTalkAction, ETriggerEvent::Started,   this, &ThisClass::Handle_PushToTalkStart);
			EIC->BindAction(PushToTalkAction, ETriggerEvent::Completed, this, &ThisClass::Handle_PushToTalkEnd);
		}

		if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
		{
			if (MoveAction) EIC->BindAction(MoveAction, ETriggerEvent::Started, TutorialSub, &UTutorialWorldSubsystem::ReportAction, EQuestConditionType::BasicAction, EQuestConditionParamType::Specific, FString("Move"));
			if (SprintAction) EIC->BindAction(SprintAction, ETriggerEvent::Started, TutorialSub, &UTutorialWorldSubsystem::ReportAction, EQuestConditionType::BasicAction, EQuestConditionParamType::Specific, FString("Run"));
			if (JumpAction) EIC->BindAction(JumpAction, ETriggerEvent::Started, TutorialSub, &UTutorialWorldSubsystem::ReportAction, EQuestConditionType::BasicAction, EQuestConditionParamType::Specific, FString("Jump"));
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
			case ELevelState::InGame:
				; // intentional fall through
				
			case ELevelState::Tutorial:
				if (InGameMappingContext) Subsystem->AddMappingContext(InGameMappingContext, 0);
				break;
			case ELevelState::Lobby:
				if (LobbyMappingContext) Subsystem->AddMappingContext(LobbyMappingContext, 0);
				break;
				
			default:
				UE_LOG(LogTemp, Warning, TEXT("[ADefaultPlayerController::HandleLevelStateChanged] Unknown Level! InGame Input applied by default."));
				if (InGameMappingContext) Subsystem->AddMappingContext(InGameMappingContext, 0);
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

void ADefaultPlayerController::Handle_Escape()
{
	const UEscapePopup* Popup = UTromboneStatics::ShowPopup<UEscapePopup>(GetWorld());
	if (!Popup)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to show escape popup"));
		return;
	}
}

void ADefaultPlayerController::Handle_CameraZoom(const FInputActionValue& Value)
{
	if (!CanProcessInput()) return;
	const float Delta = Value.Get<float>();
	if (FMath::IsNearlyZero(Delta)) return;
	CachedOwnerCharacter->OnCameraZoom(Delta);
}

void ADefaultPlayerController::Handle_Attack()
{
	if (CanProcessInput()) CachedOwnerCharacter->Attack();
}

void ADefaultPlayerController::Handle_Rhythm(const bool bPressed)
{
	if (CanProcessInput()) CachedOwnerCharacter->Rhythm(bPressed);
}

void ADefaultPlayerController::Handle_PushToTalkStart()
{
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
		{
			if (VCS->GetTalkMode() == EVoiceTalkMode::PushToTalk)
			{
				VCS->BeginLocalTalk();
				
				// UVOIPTalker::OnTalkingBegin은 듣는 사람한테만 적용되기 때문에
				// 본인이 이야기를 하고 있다를 UI로 표시하기 위해서 따로 RPC를 보내야함.
				if (ADefaultTromboneCharacter* Char = CachedOwnerCharacter.Get())
				{
					Char->Server_SetSpeaking(true);
				}
			}
		}
	}
}

void ADefaultPlayerController::Handle_PushToTalkEnd()
{
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
		{
			if (VCS->GetTalkMode() == EVoiceTalkMode::PushToTalk)
			{
				VCS->EndLocalTalk();

				// UVOIPTalker::OnTalkingEnd는 듣는 사람한테만 적용되기 때문에
				// 본인이 이야기가 끝남을 UI로 표시하기 위해서 따로 RPC를 보내야함.
				if (ADefaultTromboneCharacter* Char = CachedOwnerCharacter.Get())
				{
					Char->Server_SetSpeaking(false);
				}
			}
		}
	}
}

bool ADefaultPlayerController::CanProcessInput()
{
	return CachedOwnerCharacter.IsValid() && !CachedOwnerCharacter->IsStun() && !CachedOwnerCharacter->IsRagdoll() && CachedOwnerCharacter->IsCanProcessInput();
}
