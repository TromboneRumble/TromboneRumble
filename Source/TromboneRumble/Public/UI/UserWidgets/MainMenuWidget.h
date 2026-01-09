// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Settings/OptionPanelBase.h"
#include "MainMenuWidget.generated.h"


class UMatchMenuWidget;
class UConfirmationDialogueWidget;
class UCommonAnimatedSwitcher;
class UVideoOptionPanel;
class UAudioOptionPanel;

UCLASS()
class TROMBONERUMBLE_API UMainMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

private:
	void InitButtons();
	void ChangePanel(UWidget* TargetWidget);
	
	// ~ Begin Button Callbacks
	UFUNCTION()
	void HandleQuitButtonClicked();
	// ~ EndButton Callbacks
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ConfirmationDialogueWidgetClass;

	// ~ Start UMGs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> VB_MainMenu;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> VB_Settings;
	// ~ End UMGs
	
	// ~ Begin Common UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAudioOptionPanel> Widget_AudioOptions;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVideoOptionPanel> Widget_VideoOptions;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMatchMenuWidget> Widget_MatchMenu;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Play;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Option;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_BackFromSettings;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Quit;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Audio;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Video;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonAnimatedSwitcher> CAS_MainMenu;
	// ~ End Common UIs
	
	UPROPERTY()
	TObjectPtr<UConfirmationDialogueWidget> CachedQuitDialog;
};