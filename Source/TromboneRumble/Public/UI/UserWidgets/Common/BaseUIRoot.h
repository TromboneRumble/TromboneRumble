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
	
	// TODO : TromboneStatics로 빼기
	void PushLoadingOverlay() const;
	void PushLoadingOverlay(FString InContent) const;
	void PopLoadingOverlay() const;
	
	// TODO : TromboneStatics로 빼기
	/** Pushes fade overlay
	 * @return Fade widget that was pushed or currently active fade widget
	 * @see UFadeWidget
	 */
	UFadeWidget* PushFadeOverlay() const;
	
	/** Pops the fade overlay */
	void PopFadeOverlay() const;
	
	/** Pushes a popup widget of the specified class to the popup stack.
	 * @return The instance of the popup widget that was pushed
	 */
	UCommonActivatableWidget* PushPopup(TSubclassOf<UCommonActivatableWidget> PopupClass) const;
	
	/** Pops the topmost popup widget from the popup stack. */
	void PopPopup() const;
	
protected:
	virtual void Register();
	
protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> UIStack;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> PopupStack;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> OverlayStack;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> DefaultWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> LoadingOverlayWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> FadeWidgetClass;
};
