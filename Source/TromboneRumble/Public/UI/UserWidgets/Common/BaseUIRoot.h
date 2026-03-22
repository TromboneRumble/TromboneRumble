// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "BaseUIRoot.generated.h"

class UFadeWidget;
class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

UCLASS()
class TROMBONERUMBLE_API UBaseUIRoot : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void PushLoadingOverlay() const;
	void PushLoadingOverlay(FString InContent) const;
	void PopLoadingOverlay() const;
	
	/** Pushes fade overlay
	 * @return Fade widget that was pushed or currently active fade widget
	 * @see UFadeWidget
	 */
	UFadeWidget* PushFadeOverlay() const;
	
	/** Pops the fade overlay */
	void PopFadeOverlay() const;
	
protected:
	virtual void Register();
	
protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> UIStack;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> OverlayStack;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> DefaultWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> LoadingOverlayWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> FadeWidgetClass;
};
