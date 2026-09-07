#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Popup/PopupBase.h"
#include "EscapePopup.generated.h"

class UCommonButtonBaseExtension;
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
	virtual UWidget* GetDefaultFocusWidget() const override;
	// ~ End UPopupWidgetBase Interface

	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeOnActivated() override;
	// ~ End UCommonActivatableWidget Interface

	// ~ Begin Widgets
	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText ConfirmTitle;

	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText ConfirmDescription;

	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText ConfirmLeftButton;

	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText ConfirmRightButton;

	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText TeleportConfirmTitle;

	UPROPERTY(EditAnywhere, Category = "UI|Text")
	FText TeleportConfirmDescription;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Title;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseExtension> Button_Option;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseExtension> Button_Disconnect;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseExtension> Button_Teleport;
	// ~ End Widgets

private:

	UFUNCTION()
	void HandleOptionButtonClicked() const;

	UFUNCTION()
	void HandleDisconnectButtonClicked() const;

	UFUNCTION()
	void HandleTeleportButtonClicked();

	void RequestTeleportToResetPoint();

private:

	/**
	 * 비상탈출 확인 직후, 다음 활성화 시점에 스스로 닫기 위한 예약 플래그.
	 */
	bool bCloseOnNextActivation = false;
};
