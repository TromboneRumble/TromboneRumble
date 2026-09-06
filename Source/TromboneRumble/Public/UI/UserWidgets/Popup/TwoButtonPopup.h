#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Popup/PopupBase.h"
#include "TwoButtonPopup.generated.h"

class UCommonTextBlock;

/** Parameters for initializing the two-button popup. */
USTRUCT(BlueprintType)
struct FTwoButtonPopupParams
{
	GENERATED_BODY()

	FText Title = FText::GetEmpty();
	FText Content = FText::GetEmpty();
    
	FText LeftButtonText = FText::GetEmpty();
	FText RightButtonText = FText::GetEmpty();
    
	TFunction<void()> LeftCallback = nullptr;
	TFunction<void()> RightCallback = nullptr;
    
	bool bCloseOnLeftButtonClick = true;
	bool bCloseOnRightButtonClick = true;
};

UCLASS()
class TROMBONERUMBLE_API UTwoButtonPopup : public UPopupBase
{
	GENERATED_BODY()
	
public:
	
	/** Initializes the popup */
	void Init(const FTwoButtonPopupParams& InParams);
	
protected:
	
	// ~ Begin UPopupWidgetBase Interface
	virtual void Register() override;
	virtual void Unregister() override;
	virtual UWidget* GetDefaultFocusWidget() const override;
	// ~ End UPopupWidgetBase Interface

	// ~ Begin UI
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Title;
		
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Content;
		
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseExtension> Button_Left;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonButtonBaseExtension> Button_Right;
	// ~ End UI
	
private:
	
	UFUNCTION()
	void HandleLeftButtonClicked();
	
	UFUNCTION()
	void HandleRightButtonClicked();
	
private:
	
	/** Callback for left button */
	TFunction<void()> LeftCallback;
	
	/** Callback for right button */
	TFunction<void()> RightCallback;
	
	bool bShouldCloseOnLeftButtonClick = true;
	bool bShouldCloseOnRightButtonClick = true;
	
};