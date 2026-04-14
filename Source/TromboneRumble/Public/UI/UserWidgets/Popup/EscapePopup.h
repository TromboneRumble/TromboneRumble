#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Popup/PopupWidgetBase.h"
#include "EscapePopup.generated.h"

class UCommonButtonBaseWithText;
class UCommonTextBlock;

/**
 * Popup widget for escape menu.
 * Used in-game and tutorial.
 */
UCLASS()
class TROMBONERUMBLE_API UEscapePopup : public UPopupWidgetBase
{
	GENERATED_BODY()
	
protected:
	
	// ~ Begin UPopupWidgetBase Interface
	virtual void Register() override;
	// ~ End UPopupWidgetBase Interface
	
	// ~ Begin Widgets
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Title;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseWithText> Button_Option;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseWithText> Button_Disconnect;
	// ~ End Widgets

private:
	
	UFUNCTION()
	void HandleOptionButtonClicked();
	
	UFUNCTION()
	void HandleDisconnectButtonClicked();
};
