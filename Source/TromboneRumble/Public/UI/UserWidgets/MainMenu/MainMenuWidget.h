// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "MainMenuWidget.generated.h"

enum class EEasyMatchmakingState : uint8;
class UCommonButtonBase;
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
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	virtual void Init() override;
	virtual void SetUIEnabled(const bool bEnabled) override;
	
	virtual void BindSubsystemCallbacks() override;
	virtual void RemoveSubsystemCallbacks() override;
	
private:
	// ~ Begin Button Callbacks
	UFUNCTION()
	void HandleCreateSessionClicked();
	UFUNCTION()
	void HandleQuickJoinButtonClicked();
	UFUNCTION()
	void HandleJoinButtonClicked();
	UFUNCTION()
	void HandleQuitButtonClicked();
	// ~ End Button Callbacks
	
	UFUNCTION()
	void HandleMatchmakingUpdated(const EEasyMatchmakingState MatchmakingState, const int32 MatchmakingTime);
	
	/** Generates a random lobby code of the specified length. */
	FString GenerateRandomLobbyCode(int32 Length) const;
	
	void ShowTutorialPopup();
	
	// ~ Begin UI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_Code;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_CreateSession;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_QuickJoin;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Join;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Settings;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Tutorial;
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