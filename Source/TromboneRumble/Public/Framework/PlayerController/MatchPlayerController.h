#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MatchPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class TROMBONERUMBLE_API AMatchPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	AMatchPlayerController();

public:

	UPROPERTY(EditAnywhere, Category = "Input|Voice")
	TObjectPtr<UInputMappingContext> VoiceMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input|Voice")
	TObjectPtr<UInputAction> PushToTalkAction;

	/**
	 * Whether we want to override the view target when AutoManageActiveCameraTarget() is called.
	 * This means that a camera actor placed in the map will become the view target. By default, the first available camera will be used.
	 * This only works if bAutoManageActiveCameraTarget is enabled in the PlayerController.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby", meta = (EditCondition = "bAutoManageActiveCameraTarget"))
	bool bOverrideViewTarget;

	/** Whether we want to find a specific camera when overriding the view target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby", meta = (EditCondition = "bAutoManageActiveCameraTarget && bOverrideViewTarget"))
	bool bFindCameraByTag;

	/** The tag that we'll be looking for on camera actors when overriding the view target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby", meta = (EditCondition = "bAutoManageActiveCameraTarget && bOverrideViewTarget && bFindCameraByTag"))
	FName CameraActorTag;

protected:

	/**
	 * Whether we are about to travel to a different map or not.
	 * Keeping track of this because we don't want to override the view target when changing maps.
	 */
	bool bLeavingLobby;

protected:

	/**
	 * Called when we are overriding the view target.
	 * Attempts to find a camera actor in the game world.
	 *
	 * @return The actor that will be used as the view target. If nullptr, the default suggested view target will be used.
	 */
	virtual AActor* FindViewTargetOverride() const;

public:

	// ~ Begin APlayerController Interface
	virtual void AutoManageActiveCameraTarget(AActor* SuggestedTarget) override;
	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	// ~ End APlayerController Interface

private:
	void Handle_PushToTalkStart();
	void Handle_PushToTalkEnd();
};
