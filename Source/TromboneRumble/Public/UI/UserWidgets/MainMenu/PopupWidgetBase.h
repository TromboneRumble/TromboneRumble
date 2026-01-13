// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PopupWidgetBase.generated.h"

class UCommonButtonBase;
class UCommonLazyImage;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopupAction);

UCLASS()
class TROMBONERUMBLE_API UPopupWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	// ~ Begin Popup Options
	UPROPERTY(EditAnywhere, Category = "Popup|Option")
	bool bCloseDim = true;

	UPROPERTY(EditAnywhere, Category = "Popup|Option")
	bool bPlaySound = true;
	
	UPROPERTY(EditAnywhere, Category = "Popup|Option")
	bool bPlayAnimation = true;
	// ~ End Popup Options
	
	FOnPopupAction OnBeforeCloseAction;
	FOnPopupAction OnAfterCloseAction;
	
	virtual void Init();
	virtual void Refresh();
	
protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	
	// ~ Begin Events
	UFUNCTION()
	virtual void HandleCloseButtonClicked();
	
	UFUNCTION()
	virtual void OnCloseAnimationFinished();
	// ~ End Events
	
	virtual void ClosePopup(bool bCloseImmediately = false);
	virtual void SetEnableButtons(bool bInIsEnabled);
	
	// ~ Begin Widgets
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBase> Button_Dim;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBase> Button_Close;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeIn;
	// ~ End Widgets
	
private:
	bool bIsClosing = false;
	
};
