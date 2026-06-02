#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Popup/PopupBase.h"
#include "EscapePopup.generated.h"

class UCommonButtonBaseWithText;
class UCommonTextBlock;

/**
 * Popup widget for escape menu.
 * Used in-game and tutorial.
 */
UCLASS()
class TROMBONERUMBLE_API UEscapePopup : public UPopupBase
{
	GENERATED_BODY()
	
protected:
	
	// ~ Begin UPopupWidgetBase Interface
	virtual void Register() override;
	virtual void Unregister() override;
	// ~ End UPopupWidgetBase Interface

	// ~ Begin Widgets
	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText ConfirmTitle;

	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText ConfirmDescription;

	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText ConfirmLeftButton;

	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText ConfirmRightButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Title;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseWithText> Button_Option;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseWithText> Button_Disconnect;
	// ~ End Widgets

private:
	
	UFUNCTION()
	void HandleOptionButtonClicked() const;
	
	UFUNCTION()
	void HandleDisconnectButtonClicked() const;
};
