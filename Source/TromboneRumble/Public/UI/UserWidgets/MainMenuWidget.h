// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Utilities/Defines.h"
#include "MainMenuWidget.generated.h"

class USpinBox;
class USlider;
class UButton;
class USessionSubsystem;
class UEditableText;

UCLASS()
class TROMBONERUMBLE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual bool Initialize() override;
	virtual void NativePreConstruct() override;
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
	// ~ SessionSubsystem Callbacks
	
	UFUNCTION()
	void OnMaxPlayerSliderChanged(float Value);
	
	UFUNCTION()
	void OnMaxPlayerSpinBoxChanged(float Value);

	// Button Callbacks
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();
	// ~ Button Callbacks

	FString GenerateRandomLobbyCode(int32 Length);
	const TCHAR* JoinSessionResultToText(const EOnJoinSessionCompleteResult::Type InResult) const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> LobbyCodeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MaxPlayerSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpinBox> MaxPlayerSpinBox;
	
	int32 NumPublicConnections = 4;
	int32 MaxLobbyCodeLength{ 5 };
	EMatchState State{ EMatchState::Invalid };

	UPROPERTY(Transient)
	TObjectPtr<USessionSubsystem> SessionsSubsystem;

	UPROPERTY(Transient)
	FString CachedLobbyMapPath{ TEXT("") };
};
