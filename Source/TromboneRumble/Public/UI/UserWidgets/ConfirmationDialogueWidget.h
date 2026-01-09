// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ConfirmationDialogueWidget.generated.h"

class UCommonTextBlock;
class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UConfirmationDialogueWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	void ShowDialogue(const FText& Message);
	
protected:
	virtual void NativeOnActivated() override;
	virtual void NativeConstruct() override;
	
private:
	void InitButtons();
	void SetEnableButtons(bool bInIsEnabled);
	
	// ~ Begin Button Callbacks
	UFUNCTION()
	void HandleYesButtonClicked();
	
	UFUNCTION()
	void HandleNoButtonClicked();
	// ~ End Button Callbacks
	
	// ~ Start Common UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> MB_Yes;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> MB_No;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Message;
	// ~ End Common UIs
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeIn;
};