#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PopupBase.generated.h"

class UAkAudioEvent;
class UCommonButtonBaseWithText;
class UCommonButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopupAction);

UCLASS()
class TROMBONERUMBLE_API UPopupBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	UPopupBase();
	
	// ~ Begin Popup Options
	
	/** If true, the popup can be closed by back action (esc). */
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bAllowBackAction = true;
	
	/** If true, clicking on the dim area will close the popup. */
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bCloseDim = true;

	/** If true, the popup will play sound when opened and closed. */
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bPlaySound = true;
	
	/** If true, the popup will play animation when opened and closed. */
	UPROPERTY(EditAnywhere, Category = "Options")
	bool bPlayAnimation = true;
	
	/** The sound to play when the popup is opened */
	UPROPERTY(EditAnywhere, Category = "Options", meta = (EditCondition = "bPlaySound"))
	TObjectPtr<UAkAudioEvent> OpenSound;
	
	/** The sound to play when the popup is closed */
	UPROPERTY(EditAnywhere, Category = "Options", meta = (EditCondition = "bPlaySound"))
	TObjectPtr<UAkAudioEvent> CloseSound;
	
	// ~ End Popup Options
	
public:
	
	/** Refreshes the popup. */
	virtual void Refresh();
	
	/** Closes the popup. If bCloseImmediately is true, the popup will be closed immediately without playing the close animation. */
	virtual void ClosePopup(const bool bCloseImmediately = false);
	
public:
	
	/** @return The delegate called when the popup is opened. */
	const FOnPopupAction& OnPopupOpened() { return OnPopupOpenedEvent; }
	
	/** @return The delegate called before the popup is closed. */
	const FOnPopupAction& OnPopupClosed() { return OnPopupClosedEvent; }
	
protected:
	
	/** Registers the widget events. e.g. button click events. */
	virtual void Register();
	
	/** Unregisters the widget events. e.g. button click events. */
	virtual void Unregister();
	
private:
	
	/** Event when the popup is opened. Called after open animation is finished. */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Popup Opened", meta = (AllowPrivateAccess))
	FOnPopupAction OnPopupOpenedEvent;
	
	/** Event when the popup is closed. Called before close animation is started */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Popup Closed", meta = (AllowPrivateAccess))
	FOnPopupAction OnPopupClosedEvent;
	
	/** is popup currently closing? */
	bool bIsClosing = false;
	
protected:
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	virtual bool NativeOnHandleBackAction() override;
	// ~ End UCommonActivatableWidget Interface
	
	// ~ Begin Widgets
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBase> Button_Dim;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseWithText> Button_Close;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeIn;
	// ~ End Widgets
	
};
