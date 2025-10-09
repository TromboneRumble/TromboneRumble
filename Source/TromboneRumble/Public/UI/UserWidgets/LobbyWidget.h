// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbyWidget.generated.h"

enum class ELobbyState : uint8;
class UButton;
class USessionSubsystem;
class UTextBlock;

UCLASS()
class TROMBONERUMBLE_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// SessionSubsystem Callbacks
	void BindSubsystemCallbacks();
	void RemoveSubsystemCallbacks();

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnSessionError(const FString& Reason);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	UFUNCTION()
	void OnPlayerListUpdated(const TArray<FString>& PlayerNames);
	// ~ SessionSubsystem Callbacks

	// Button Callbacks
	UFUNCTION()
	void StartGameButtonClicked();

	UFUNCTION()
	void BackToMainMenuButtonClicked();
	// ~ Button Callbacks

	UFUNCTION()
	void OnLobbyStateUpdated(ELobbyState NewState);

	void UpdateCountdown();


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackToMainMenuButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LobbyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> IsHostText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerListText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountdownText;

	UPROPERTY(Transient)
	TObjectPtr<USessionSubsystem> SessionsSubsystem;

	UPROPERTY(Transient)
	FString CachedMainMenuMapPath{ TEXT("") };

	UPROPERTY(Transient)
	FString CachedInGameMapPath{ TEXT("") };

	FTimerHandle CountdownTimerHandle;
	int32 CountdownSeconds = 5;
};
