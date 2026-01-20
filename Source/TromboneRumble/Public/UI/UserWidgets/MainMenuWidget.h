// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainMenu/BaseMenuWidget.h"
#include "Settings/OptionPanelBase.h"
#include "MainMenuWidget.generated.h"

enum class EMainMenuType : uint8;
class UEasySessionSubsystem;
class UEditableText;
class UMatchMenuWidget;
class UConfirmationDialogueWidget;
class UCommonAnimatedSwitcher;
class UVideoOptionPanel;
class UAudioOptionPanel;

UCLASS()
class TROMBONERUMBLE_API UMainMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()
	
protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	virtual void NativeConstruct() override;
	virtual void Init() override;
	virtual void SetUIEnabled(const bool bEnabled) override;
	
	virtual void BindSubsystemCallbacks() override;
	virtual void RemoveSubsystemCallbacks() override;
	
private:
	// ~ Begin Button Callbacks
	UFUNCTION()
	void HandleOnlineButtonClicked();
	UFUNCTION()
	void HandleJoinButtonClicked();
	UFUNCTION()
	void HandleQuitButtonClicked();
	// ~ End Button Callbacks
	
	void OnStartSessionSuccess();
	void OnStartSessionFailure();
	
	void OnFindSessionsSuccess(const TArray<FOnlineSessionSearchResult>& SessionResults);
	void OnFindSessionsFailure(const TArray<FOnlineSessionSearchResult>& SessionResults);
    
	void OnJoinSessionSuccess();
	void OnJoinSessionFailure();
	
	void OnDestroySessionSuccess();
	void OnDestroySessionFailure();
	
	FString GenerateRandomLobbyCode(int32 Length) const;
	void StartHostValidation(const FString& Code);
	void CreateSessionAfterValidation(const FString& ValidatedCode);
	bool bIsSearchingForHostValidation = false;
	FString PendingLobbyCode;
	
	// ~ Begin UI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_Code;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Online;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Join;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Settings;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Guide;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Quit;
	// ~ End UI
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ConfirmationDialogueWidgetClass;
	UPROPERTY()
	TObjectPtr<UConfirmationDialogueWidget> CachedQuitDialog;
	
	UPROPERTY(Transient)
	FString CachedMatchMenuMapPath = "";
};