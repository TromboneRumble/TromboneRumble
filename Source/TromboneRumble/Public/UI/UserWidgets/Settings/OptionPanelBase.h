// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "OptionPanelBase.generated.h"

class UCommonTextBlock;
class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UOptionPanelBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void Init(TFunction<void()> BackAction); 
	
protected:
	virtual void InitButtons();
	
	virtual void HandleBackButtonClicked();
	
	TFunction<void()> OnBackAction;
	
	// ~ Begin Common UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_OptionPanelTitle;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Back;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Apply;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Reset;
	// ~ End Common UIs
};
