// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/LoadingOverlayWidget.h"
#include "CommonTextBlock.h"

void ULoadingOverlayWidget::InitWithContent(const FText& InContent)
{
	const FText FinalContent = InContent.IsEmpty() ? DefaultContent : InContent;
	
	if (CT_Content)
	{
		CT_Content->SetText(FinalContent);
	}
}
