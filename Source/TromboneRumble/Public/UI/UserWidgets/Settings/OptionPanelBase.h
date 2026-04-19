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

	/** Refreshes the option panel UI to reflect the current settings. */
	virtual void RefreshUI();
	
protected:
	
	/** Activates the option panel */
	virtual void Activate();
	
	/** Deactivates the option panel */
	virtual void Deactivate();
	
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
