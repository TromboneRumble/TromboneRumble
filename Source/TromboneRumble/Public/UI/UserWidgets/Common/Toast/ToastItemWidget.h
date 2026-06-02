// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Data/UIData.h"
#include "ToastItemWidget.generated.h"

UCLASS()
class TROMBONERUMBLE_API UToastItemWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor */
	UToastItemWidget();
	
	/** Initialize the toast widget with the given request data. This will also play the open tween. */
	void InitializeToast(const FToastRequest& Request, const FSimpleDelegate& InToastFinishedCallback = FSimpleDelegate());
	
	/** Deinitialize the toast widget */
	void DeinitializeToast();
	
	/** Immediately close the toast without waiting for the display duration. Using in Override policy */
	void CloseToastImmediately();

protected:
	
	/** Blueprint event called when the toast is initialized. Use this to set up the toast message */
	UFUNCTION(BlueprintImplementableEvent, Category = "Toast", DisplayName = "On Toast Initialized")
	void K2_OnToastInitialized(const FToastRequest& Request);

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Toast|Animation")
	float AnimInDuration;

	UPROPERTY(EditDefaultsOnly, Category = "Toast|Animation")
	float AnimOutDuration;
	
private:
	
	void StartTween(const EToastTweenType TweenType, const float InDuration);
	
	void HandleAnimInTweenFinished();
	
	void HandleAnimOutTweenFinished();
	
	FTimerHandle TimerHandle_StartCloseTween;
	FSimpleDelegate OnToastFinishedCallback;
	float ToastDisplayDuration;
	
public:
	
	// ~ Begin UUserWidget Interface
	virtual void NativeDestruct() override;
	// ~ End UUserWidget Interface
	
};
