#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PopupWidgetBase.generated.h"

class UCommonButtonBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FK2_OnPopupAction);

UCLASS()
class TROMBONERUMBLE_API UPopupWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	/** Default constructor. */
	UPopupWidgetBase();
	
	// ~ Begin Popup Options
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bCloseDim;

	UPROPERTY(EditAnywhere, Category = "Options")
	bool bPlaySound;
	
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bPlayAnimation;
	// ~ End Popup Options
	
public:
	
	/** Refreshes the popup. */
	virtual void Refresh();
	
	/** Closes the popup. If bCloseImmediately is true, the popup will be closed immediately without playing the close animation. */
	virtual void ClosePopup(bool bCloseImmediately = false);
	
public:
	
	/** @return The delegate called when the popup is opened. */
	FK2_OnPopupAction OnPopupOpened() { return OnPopupOpenedEvent; }
	
	/** @return The delegate called before the popup is closed. */
	FK2_OnPopupAction OnPopupClosed() { return OnPopupClosedEvent; }
	
protected:
	
	/** Registers the widget events. e.g. button click events. */
	virtual void Register();
	
private:
	
	/** Event when the popup is opened. Called after open animation is finished. */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Popup Opened", meta = (AllowPrivateAccess))
	FK2_OnPopupAction OnPopupOpenedEvent;
	
	/** Event when the popup is closed. Called before close animation is started */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Popup Closed", meta = (AllowPrivateAccess))
	FK2_OnPopupAction OnPopupClosedEvent;
	
	/** Is the popup currently in the process of closing */
	bool bIsClosing;
	
protected:
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	// ~ End UCommonActivatableWidget Interface
	
	// ~ Begin Widgets
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBase> Button_Dim;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBase> Button_Close;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeIn;
	// ~ End Widgets
	
};
