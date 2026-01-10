// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Utilities/Defines.h"
#include "MatchMenuWidget.generated.h"

class UEasySessionSubsystem;
class UCommonButtonBase;
class UEditableText;
class UButton;
class USlider;
class USpinBox;

UCLASS()
class TROMBONERUMBLE_API UMatchMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual void Init(TFunction<void()> OnMenuClosedCallback);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	
	virtual void InitButtons();
	
	TFunction<void()> OnMenuClosed;
	
private:
	// ~ Begin SessionSubsystem Callbacks
	void BindSubsystemCallbacks();
	void RemoveSubsystemCallbacks();
	
	void OnStartSessionSuccess();
	void OnStartSessionFailure();
    
	void OnFindSessionsSuccess(const TArray<FOnlineSessionSearchResult>& SessionResults);
	void OnFindSessionsFailure(const TArray<FOnlineSessionSearchResult>& SessionResults);
    
	void OnJoinSessionSuccess();
	void OnJoinSessionFailure();
	
	void OnDestroySessionSuccess();
	void OnDestroySessionFailure();
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
	// ~ EndButton Callbacks
	
	void MatchButtonsSetEnabled(const bool bEnabled);
	FString GenerateRandomLobbyCode(int32 Length) const;
	const TCHAR* JoinSessionResultToText(const EOnJoinSessionCompleteResult::Type InResult) const;
	
	// ~ Begin UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Back;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Host;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Join;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> LobbyCodeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MaxPlayerSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpinBox> MaxPlayerSpinBox;
	// ~ End UIs
	
	int32 MaxLobbyCodeLength = 5;
	EMatchState State = EMatchState::Invalid;

	UPROPERTY(Transient)
	TObjectPtr<UEasySessionSubsystem> SessionsSubsystem;

	UPROPERTY(Transient)
	FString CachedLobbyMapPath = "";
};