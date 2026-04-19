#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Popup/PopupWidgetBase.h"
#include "TwoButtonWithoutClosePopup.generated.h"

class UCommonButtonBaseWithText;
class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UTwoButtonWithoutClosePopup : public UPopupWidgetBase
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	UTwoButtonWithoutClosePopup();
	
	/** Initializes the popup */
	void OnInit(const FText& InTitle, const FText& InContent, const FText& InLeftText, const FText& InRightText, TFunction<void()> InLeftCallback, TFunction<void()> InRightCallback, const bool bShouldClosePopup = true);
	
protected:
	
	/** Callback for left button */
	TFunction<void()> LeftCallback;
	
	/** Callback for right button */
	TFunction<void()> RightCallback;
	
	/** If true, the popup will be closed after clicking any button. */
	bool bShouldClosePopupAfterClick;
	
protected:
	
	// ~ Begin UPopupWidgetBase Interface
	virtual void Register() override;
	// ~ End UPopupWidgetBase Interface
	
	// ~ Begin UI
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Title;
		
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Content;
		
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseWithText> Button_Left;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseWithText> Button_Right;
	// ~ End UI
	
private:
	
	UFUNCTION()
	void HandleLeftButtonClicked();
	
	UFUNCTION()
	void HandleRightButtonClicked();
	
};