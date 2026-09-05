#include "Framework/PlayerController/MatchPlayerController.h"
#include "Subsystems/TromboneUISubsystem.h"
#include "Camera/CameraActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Pawns/MatchPawn.h"
#include "Subsystems/VoiceChatSubsystem.h"
#include "UObject/UObjectIterator.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

AMatchPlayerController::AMatchPlayerController()
{
	bOverrideViewTarget = true;
	bFindCameraByTag = false;
	CameraActorTag = NAME_None;
	bLeavingLobby = false;
}

void AMatchPlayerController::AutoManageActiveCameraTarget(AActor* SuggestedTarget)
{
	if (bOverrideViewTarget && !bLeavingLobby)
	{
		if (AActor* ViewTargetOverride = FindViewTargetOverride())
		{
			SuggestedTarget = ViewTargetOverride;
		}
	}

	Super::AutoManageActiveCameraTarget(SuggestedTarget);
}

AActor* AMatchPlayerController::FindViewTargetOverride() const
{
	for (TObjectIterator<ACameraActor> It; It; ++It)
	{
		if (ACameraActor* CameraActor = *It)
		{
			if (bFindCameraByTag && !CameraActor->ActorHasTag(CameraActorTag))
			{
				continue;
			}

			if (CameraActor->GetWorld() != GetWorld())
			{
				// In some cases the camera actors world might not be the same as the player controllers world.
				// This was happening during net startup when running multiple clients in the editor.
				// We need to stop here otherwise the game would crash.
				// This function is called multiple times so the camera will be found eventually.
				continue;
			}

			// Found a good view target.
			return CameraActor;
		}
	}

	UE_CLOG(bFindCameraByTag, LogTemp, Warning, TEXT("No Camera found with tag '%s.' Failed to override view target"), *CameraActorTag.ToString());
	return nullptr;
}

void AMatchPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
		{
			VCS->EnsureLocalTalkerRegistered();
			if (VCS->GetTalkMode() == EVoipMode::AutoVoice)
				VCS->BeginLocalTalk();
			else
				VCS->EndLocalTalk();
		}

		if (CommonMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(CommonMappingContext, 0);
			}
		}
	}
}

void AMatchPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PushToTalkAction)
		{
			EIC->BindAction(PushToTalkAction, ETriggerEvent::Started,   this, &ThisClass::Handle_PushToTalkStart);
			EIC->BindAction(PushToTalkAction, ETriggerEvent::Completed, this, &ThisClass::Handle_PushToTalkEnd);
		}
	}
}

void AMatchPlayerController::Handle_PushToTalkStart()
{
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
		{
			if (VCS->GetTalkMode() == EVoipMode::PushToTalk)
			{
				VCS->BeginLocalTalk();
				
				// UVOIPTalker::OnTalkingBegin은 듣는 사람한테만 적용되기 때문에
				// 본인이 이야기를 하고 있다를 UI로 표시하기 위해서 따로 RPC를 보내야함.
				if (AMatchPawn* MP = GetPawn<AMatchPawn>())
				{
					MP->Server_SetSpeaking(true);
				}
			}
		}
	}
}

void AMatchPlayerController::Handle_PushToTalkEnd()
{
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
		{
			if (VCS->GetTalkMode() == EVoipMode::PushToTalk)
			{
				VCS->EndLocalTalk();

				// UVOIPTalker::OnTalkingEnd는 듣는 사람한테만 적용되기 때문에
				// 본인이 이야기가 끝남을 UI로 표시하기 위해서 따로 RPC를 보내야함.
				if (AMatchPawn* MP = GetPawn<AMatchPawn>())
				{
					MP->Server_SetSpeaking(false);
				}
			}
		}
	}
}

void AMatchPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (UTromboneUISubsystem* UISubsystem = UTromboneUISubsystem::Get(this))
	{
		UISubsystem->AttachRootUI(this);
	}
#if !UE_BUILD_SHIPPING
	if (!CheatManager)
		AddCheats(true);
#endif
}

void AMatchPlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);

	bLeavingLobby = true;

	if (bIsSeamlessTravel)
	{
		if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
		{
			ViewportClient->RemoveAllViewportWidgets();
		}
	}
}
