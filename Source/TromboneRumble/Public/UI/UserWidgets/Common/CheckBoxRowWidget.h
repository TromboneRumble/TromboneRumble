// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CheckBoxRowWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UCheckBoxRowWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_CheckBox;

public:
	
	/** Sets the checkbox state. */
	void SetChecked(bool bChecked);
	
	/** @return The checkbox state. */
	bool IsChecked() const;

};
