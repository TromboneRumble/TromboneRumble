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
	
	/** Should automatically reapply currently activated option panel's 'saved' data on deactivate? */
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bAutoReapplySettingsOnDeactivate = true;
	
public:
	
	/** Synchronize ui from saved data */
	virtual void RefreshUI();

	/**
	 * Apply the values from the UI to the actual settings.
	 * @param bSaveToDisk if true, changes will be saved to the disk.
	 */
	virtual void ApplySettingsFromUI(bool bSaveToDisk = true);
	
	/** Apply the values from the saved data */
	virtual void ApplySettingsFromSavedData();
	
	/** @return Whether there are any changes in this options panel */
	virtual bool IsDirty() const { return false; }
	
protected:
	
	/** Registers the widget events. e.g. button click events. */
	virtual void Register();
	
	/** Unregisters the widget events. e.g. button click events. */
	virtual void Unregister();
	
protected:
	
	UPROPERTY()
	TObjectPtr<USaveManagerSubsystem> SaveManagerSubsystem;

public:
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~ End UCommonActivatableWidget Interface
	
};
