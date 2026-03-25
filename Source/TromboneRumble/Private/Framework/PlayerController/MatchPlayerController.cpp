#include "Framework/PlayerController/MatchPlayerController.h"
#include "Camera/CameraActor.h"
#include "UObject/UObjectIterator.h"
#include "Engine/GameViewportClient.h"
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
