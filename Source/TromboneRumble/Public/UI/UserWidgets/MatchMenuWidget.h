// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Utilities/Defines.h"
#include "MatchMenuWidget.generated.h"

class UCommonButtonBase;
class UEditableText;
class UButton;
class USlider;
class USpinBox;
class USessionSubsystem;

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
	// ~ EndButton Callbacks
	
	FString GenerateRandomLobbyCode(int32 Length);
	const TCHAR* JoinSessionResultToText(const EOnJoinSessionCompleteResult::Type InResult) const;
	
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
	
	int32 NumPublicConnections = 4;
	int32 MaxLobbyCodeLength = 4;
	EMatchState State = EMatchState::Invalid;

	UPROPERTY(Transient)
	TObjectPtr<USessionSubsystem> SessionsSubsystem;

	UPROPERTY(Transient)
	FString CachedLobbyMapPath = "";
};