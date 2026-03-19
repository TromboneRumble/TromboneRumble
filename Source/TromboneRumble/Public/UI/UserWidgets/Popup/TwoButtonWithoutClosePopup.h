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
	
	/** Initializes the popup */
	virtual void OnInit(const FText& InTitle, const FText& InContent, const FText& LeftText, const FText& RightText, const FOnPopupAction& InLeftButtonDelegate, const FOnPopupAction& InRightButtonDelegate);
		
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void HandleLeftButtonClicked();
	
	UFUNCTION()
	void HandleRightButtonClicked();
	
protected:
	
	FOnPopupAction OnLeftButtonClicked;
	FOnPopupAction OnRightButtonClicked;
	
protected:
	
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
	
};