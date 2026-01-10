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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged, APlayerState*, NewPlayerState);

UCLASS()
class TROMBONERUMBLE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	// InputActions
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> LobbyMappingContext;
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
	// ~InputActions

	UPROPERTY(BlueprintAssignable, Category = "PlayerState")
	FOnPlayerStateChanged OnPlayerStateChanged;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void AcknowledgePossession(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void HandleGameStateChanged(EGameState NewState);

	UFUNCTION()
	void HandlePlayerStateAdded(APlayerState* InPlayerState);

	UFUNCTION()
	void HandlePlayerStateRemoved(APlayerState* InPlayerState);

	UFUNCTION()
	void HandleOnLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UOSI_RhythmRankWidget> RhythmRankWidgetClass;

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, TObjectPtr<UOSI_RhythmRankWidget>> PlayerStateToRhythmRankWidgetMap;
private:
	// Input handlers
	void Handle_Move(const struct FInputActionValue& Value);
	void Handle_JumpPressed();
	void Handle_JumpReleased();
	void Handle_Interact();
	void Handle_SprintPressed();
	void Handle_SprintReleased();
	void Handle_Attack();
	void Handle_Rhythm(bool bPressed);
	// ~Input handlers

	bool CanProcessInput();
	
	UFUNCTION(Server, Reliable)
	void Server_NotifyLoadingScreenFinished();

	void HandleLoadingScreenFinished();

	bool bHasNotifiedLoadingFinished = false;

	FTimerHandle RetryCreateRankWidgetsHandle;
	bool bRetryTimerRunning = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultTromboneCharacter> CachedOwnerCharacter = nullptr;
};
