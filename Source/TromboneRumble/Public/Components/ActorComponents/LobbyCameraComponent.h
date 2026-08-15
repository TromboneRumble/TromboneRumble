// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utilities/Defines.h"
#include "LobbyCameraComponent.generated.h"

class ACameraActor;
class ALobbyGameState;

/** ULobbyCameraComponent
 *
 *  Client-only component on the PlayerController that plays the lobby intro camera.
 *  Views a camera actor placed in the map until every player has landed, then blends to the owning character.
 *  Stays inactive outside lobby levels and for players who join after the intro is over.
 */
UCLASS()
class TROMBONERUMBLE_API ULobbyCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "LobbyCamera", meta = (DisplayName = "로비 연출 카메라 액터 태그"))
	FName CameraActorTag = TEXT("LobbyCenterCamera");

	UPROPERTY(EditAnywhere, Category = "LobbyCamera", meta = (DisplayName = "내 캐릭터로 전환 블렌드 시간(초)"))
	float BlendTime = 1.5f;

public:

	/** Default Constructor */
	ULobbyCameraComponent();

	/** @return the lobby camera while the intro is not over yet, so the owner can suggest it over the pawn. Null otherwise.
	 *  Finds the camera on demand instead of waiting for the intro to start, so it works from the very first frame */
	AActor* GetViewTargetOverride();

public:

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

private:

	/** Starts the intro as soon as the local player and the lobby camera exist, retrying on the next tick until then. */
	void TryStartDirecting(int32 RetryCount);

	/** Decides from the replicated lobby state whether to keep the intro or blend to the pawn right away. */
	void ResolveLobbyState(int32 RetryCount);

	/** Blends the view to the owning pawn and ends the intro, retrying on the next tick until the pawn exists. */
	void BlendToOwnPawn(int32 RetryCount);

	/** Ends the intro and unsubscribes from every delegate. */
	void StopDirecting();

	/** Finds the camera actor that has CameraActorTag. */
	ACameraActor* FindCenterCamera() const;

	/** @return true if not every player has landed yet. */
	static bool IsPreGroundedState(ELobbyState State);

	UFUNCTION()
	void HandleLobbyStateChanged(ELobbyState NewState);

private:

	UPROPERTY()
	TObjectPtr<APlayerController> OwnerPC;

	UPROPERTY()
	TObjectPtr<ALobbyGameState> LobbyGameState;

	UPROPERTY()
	TObjectPtr<ACameraActor> CenterCamera;

	/** Whether the intro camera is running right now. */
	bool bDirecting = false;

	/** Whether the intro is over on this controller. Blocks any further view target override. */
	bool bIntroFinished = false;

	/** How many next-tick retries are allowed while waiting for the local player, camera, game state, or pawn. */
	static constexpr int32 MaxRetryCount = 300;

};
