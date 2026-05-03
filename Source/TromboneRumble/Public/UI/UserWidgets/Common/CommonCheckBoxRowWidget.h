// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CommonCheckBoxRowWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UCommonCheckBoxRowWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_CheckBox;

public:
	
	/** Sets the checkbox state. */
	UFUNCTION(BlueprintCallable, Category = "CheckBox")
	void SetChecked(bool bChecked);
	
	/** @return The checkbox state. */
	UFUNCTION(BlueprintPure, Category = "CheckBox")
	bool IsChecked() const;

};
