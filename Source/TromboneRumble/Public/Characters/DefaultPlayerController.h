// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Utilities/Defines.h"
#include "DefaultPlayerController.generated.h"

class UOSI_RhythmRankWidget;
class UAkAudioEvent;
class UInteractorComponent;
class UInGameWidget;
class UInputMappingContext;
class UInputAction;
class ADefaultTromboneCharacter;
class ULobbyCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged, APlayerState*, NewPlayerState);

UCLASS()
class TROMBONERUMBLE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	
	/** Default constructor. */
	ADefaultPlayerController();

	// InputActions
	/** Every level gets this one. Holds the keys the lobby and the match share */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> CommonMappingContext;
	
	/** Added on top of the common one while a match is running. Jump, Rhythm, Escape */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> InGameMappingContext;
	
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> SprintAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> AttackAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> RhythmAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> EscapeAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> CameraZoomAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> PushToTalkAction;
	// ~InputActions

	UPROPERTY(BlueprintAssignable, Category = "PlayerState")
	FOnPlayerStateChanged OnPlayerStateChanged;

	/** Lobby director camera (local only; run only on the lobby level) */
	UPROPERTY(VisibleAnywhere, Category = "Lobby")
	TObjectPtr<ULobbyCameraComponent> LobbyCameraComponent;

	//~ Begin APlayerController Interface
	/** Suggests the lobby intro camera over the pawn while the intro is running */
	virtual void AutoManageActiveCameraTarget(AActor* SuggestedTarget) override;

	/** Redirects server-driven view target changes to the lobby intro camera while the intro is running */
	virtual void ClientSetViewTarget_Implementation(AActor* A, FViewTargetTransitionParams TransitionParams) override;
	//~ End APlayerController Interface

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void AcknowledgePossession(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void HandleLevelStateChanged(ELevelType NewState);

	UFUNCTION()
	void HandlePlayerStateAdded(APlayerState* InPlayerState);

	UFUNCTION()
	void HandlePlayerStateRemoved(APlayerState* InPlayerState);

	UFUNCTION()
	void HandleOnLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader);
	
	UFUNCTION()
	void HandleInGameStateChanged(EInGameState NewState);

	UFUNCTION()
	void HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState);

	UFUNCTION(Server, Reliable)
	void Server_RhythmGameFinished();
	
public:
	
	UFUNCTION(Server, Reliable)
	void Server_ReportClientTravelToResultLevelAndLeaveSession();
	
	UFUNCTION(Client, Reliable)
	void Client_RequestTravelToResultLevelAndLeaveSession();

protected:

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UOSI_RhythmRankWidget> RhythmRankWidgetClass;

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, TObjectPtr<UOSI_RhythmRankWidget>> PlayerStateToRhythmRankWidgetMap;
	UFUNCTION(BlueprintCallable)
	void Handle_Attack();
	UFUNCTION(BlueprintCallable)
	void Handle_Rhythm(bool bPressed);
private:
	// Input handlers
	void Handle_Move(const struct FInputActionValue& Value);
	void Handle_JumpPressed();
	void Handle_JumpReleased();
	void Handle_Interact();
	void Handle_SprintPressed();
	void Handle_SprintReleased();
	void Handle_Escape();
	void Handle_CameraZoom(const struct FInputActionValue& Value);
	void Handle_PushToTalkStart();
	void Handle_PushToTalkEnd();
	// ~Input handlers

	bool CanProcessInput();
	
	UFUNCTION(Server, Reliable)
	void Server_NotifyLoadingScreenFinished();

	void HandleLoadingScreenFinished();

	bool bHasNotifiedLoadingFinished = false;

	UFUNCTION(Server, Reliable)
	void Server_NotifyLoadingFinishedToInGameMode();

	// 세션을 부수고 결과 레벨로 이동한다. 서버 보고가 나간 다음 틱에 부른다
	void LeaveSessionAndTravelToResultLevel();

	FTimerHandle RetryCreateRankWidgetsHandle;
	bool bRetryTimerRunning = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultTromboneCharacter> CachedOwnerCharacter = nullptr;
};
