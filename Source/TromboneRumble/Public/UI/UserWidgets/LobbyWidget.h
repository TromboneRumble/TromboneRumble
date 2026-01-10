// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

class UEasySessionSubsystem;
enum class ELobbyState : uint8;
class UButton;
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
	// ~ Begin GameState Events
	void BindGameStateEvents();
	void RemoveGameStateEvents();

	UFUNCTION()
	void OnPlayerListChanged(const TArray<FString>& PlayerNames);
	// ~ End GameState Events
	
	// ~ Begin SessionSubsystem Callbacks
	void BindSubsystemCallbacks();
	void RemoveSubsystemCallbacks();

	void OnDestroySessionSuccess();
	void OnDestroySessionFailure();
	// ~ End SessionSubsystem Callbacks

	// ~ Begin Button Callbacks
	UFUNCTION()
	void StartGameButtonClicked();

	UFUNCTION()
	void BackToMainMenuButtonClicked();
	// ~ End Button Callbacks

	UFUNCTION()
	void OnLobbyStateUpdated(ELobbyState NewState);

	void UpdateCountdown();

	// ~ Begin UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LobbyText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> IsHostText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerListText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountdownText;
	// ~ End UIs

	UPROPERTY(Transient)
	TObjectPtr<UEasySessionSubsystem> SessionsSubsystem;

	UPROPERTY(Transient)
	FString CachedMainMenuMapPath = TEXT("");
	UPROPERTY(Transient)
	FString CachedInGameMapPath = TEXT("");
	
	FTimerHandle CountdownTimerHandle;
	int32 CountdownSeconds = 5;
};