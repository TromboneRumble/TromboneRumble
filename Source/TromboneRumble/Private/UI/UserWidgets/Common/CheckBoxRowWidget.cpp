// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/CheckBoxRowWidget.h"
#include "CommonButtonBase.h"

void UCheckBoxRowWidget::SetChecked(bool bChecked)
{
	CB_CheckBox->SetIsSelected(bChecked);
}

bool UCheckBoxRowWidget::IsChecked() const
{
	return CB_CheckBox->GetSelected();
}
