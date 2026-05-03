// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "OptionPanelBase.generated.h"

class USaveManagerSubsystem;
class UCommonTextBlock;
class UCommonButtonBase;

UCLASS(Abstract)
class TROMBONERUMBLE_API UOptionPanelBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	
	/** Should automatically Refresh UI on activate? */
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bAutoRefreshUIOnActivate = true;
	
	/** Should automatically reapply currently activated option panel's 'saved' settings on deactivate? */
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bAutoReapplySettingsOnDeactivate = true;
	
protected:
	
	/** Apply 'saved' settings to UI */
	virtual void RefreshUI();
	
	/** Apply 'saved' settings. does not affect the UI */
	virtual void ReapplySavedSettings();
	
	/** Registers the widget events. e.g. button click events. */
	virtual void Register();
	
	/** Unregisters the widget events. e.g. button click events. */
	virtual void Unregister();
	
	virtual void HandleApplyButtonClicked();
	virtual void HandleResetButtonClicked();
	
protected:
	
	UPROPERTY()
	TObjectPtr<USaveManagerSubsystem> SaveManagerSubsystem;
	
	// ~ Begin Common UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Apply;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Reset;
	// ~ End Common UIs
	
public:
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~ End UCommonActivatableWidget Interface
	
};
