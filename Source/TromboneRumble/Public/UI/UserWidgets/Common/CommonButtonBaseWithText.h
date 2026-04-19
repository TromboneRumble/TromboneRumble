#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonButtonBaseWithText.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UCommonButtonBaseWithText : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	/** Sets the button text. */
	void SetText(const FText& InText) const;

protected:
	
	/** Text block to display the button text. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_ButtonText;
	
protected:
	
	// ~ Begin UCommonButtonBase Interface
	virtual void NativeOnPressed() override;
	virtual void NativeOnReleased() override;
	// ~ End UCommonButtonBase Interface
	
protected:
	
	/** if NormalPressedTextStyle is applied, OnReleased will reset the text style to NormalTextStyle */
	bool bIsNormalPressedTextStyleApplied = false;
	
};
