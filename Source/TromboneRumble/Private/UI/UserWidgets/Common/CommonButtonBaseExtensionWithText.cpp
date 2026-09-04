// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/CommonButtonBaseExtensionWithText.h"
#include "CommonTextBlock.h"

void UCommonButtonBaseExtensionWithText::SetText(const FText& InText)
{
	if (Text_ActionName)
	{
		Text_ActionName->SetText(InText);
	}
}

void UCommonButtonBaseExtensionWithText::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	
	if (Text_ActionName)
	{
		Text_ActionName->SetStyle(GetDesiredTextStyleClass());
	}
}
