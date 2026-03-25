// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "OptionPanelBase.generated.h"

class USaveManagerSubsystem;
class UCommonTextBlock;
class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UOptionPanelBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void Deactivate();
	
protected:
	
	virtual void Init(); 
	virtual void InitButtons();
	
	virtual void HandleApplyButtonClicked();
	virtual void HandleResetButtonClicked();
	
	UPROPERTY()
	TObjectPtr<USaveManagerSubsystem> SaveManagerSubsystem;
	
	// ~ Begin Common UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Apply;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Reset;
	// ~ End Common UIs
	
public:
	
	// ~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	// ~ End UUserWidget Interface
};
