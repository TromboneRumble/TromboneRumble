// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CommonButtonBase.h"
#include "CheckBoxRowWidget.generated.h"

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

	/** Exposes the underlying button click event for external binding. */
	UCommonButtonBase::FCommonButtonEvent& OnClicked() const { return CB_CheckBox->OnClicked(); }

	/** @return The inner widget that should receive gamepad/keyboard focus. */
	UWidget* GetFocusWidget() const { return CB_CheckBox; }

};
