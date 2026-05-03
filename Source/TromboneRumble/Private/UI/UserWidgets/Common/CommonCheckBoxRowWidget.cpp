// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/CommonCheckBoxRowWidget.h"
#include "CommonButtonBase.h"

void UCommonCheckBoxRowWidget::SetChecked(bool bChecked)
{
	CB_CheckBox->SetIsSelected(bChecked);
}

bool UCommonCheckBoxRowWidget::IsChecked() const
{
	return CB_CheckBox->GetSelected();
}
