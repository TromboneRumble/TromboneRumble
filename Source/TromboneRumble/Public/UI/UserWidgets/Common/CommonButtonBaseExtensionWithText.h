// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/CommonButtonBaseExtension.h"
#include "CommonButtonBaseExtensionWithText.generated.h"

class UCommonTextBlock;

/**
 * UCommonButtonStyleExtension에서 텍스트 하나를 가진 확장 버튼.
 */
UCLASS()
class TROMBONERUMBLE_API UCommonButtonBaseExtensionWithText : public UCommonButtonBaseExtension
{
	GENERATED_BODY()
	
public:
	
	/** Sets the button text. */
	void SetText(const FText& InText);
	
protected:
	
	//~ Begin UCommonButtonBase Interface
	virtual void NativeOnCurrentTextStyleChanged() override;
	//~ End UCommonButtonBase Interface
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Text Block")
	TObjectPtr<UCommonTextBlock> Text_ActionName;
	
};
