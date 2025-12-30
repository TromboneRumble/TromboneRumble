// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Utilities/Defines.h"
#include "MainMenuWidget.generated.h"

class UVerticalBox;
class UCommonAnimatedSwitcher;
class UCommonButtonBase;
class USpinBox;
class USlider;
class UButton;
class USessionSubsystem;
class UEditableText;

UCLASS()
class TROMBONERUMBLE_API UMainMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual bool Initialize() override;
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

private:
	void InitButtons();
	void ChangePanel(UVerticalBox* TargetPanel);
	
	// ~ Begin SessionSubsystem Callbacks
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
	// ~ End SessionSubsystem Callbacks
	
	UFUNCTION()
	void OnMaxPlayerSliderChanged(float Value);
	
	UFUNCTION()
	void OnMaxPlayerSpinBoxChanged(float Value);

	// ~ Begin Button Callbacks
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();
	
	UFUNCTION()
	void HandleOptionButtonClicked();
	UFUNCTION()
	void HandleBackFromSettingsButtonClicked();
	// ~ EndButton Callbacks

	FString GenerateRandomLobbyCode(int32 Length);
	const TCHAR* JoinSessionResultToText(const EOnJoinSessionCompleteResult::Type InResult) const;

	// ~ Start UMGs
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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_MainMenu;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_Settings;
	// ~ End UMGs
	
	// ~ Start Common UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> MB_Option;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> MB_BackFromSettings;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonAnimatedSwitcher> CAS_MainMenu;
	// ~ End Common UIs
	
	int32 NumPublicConnections = 4;
	int32 MaxLobbyCodeLength{ 5 };
	EMatchState State{ EMatchState::Invalid };

	UPROPERTY(Transient)
	TObjectPtr<USessionSubsystem> SessionsSubsystem;

	UPROPERTY(Transient)
	FString CachedLobbyMapPath{ TEXT("") };
};
