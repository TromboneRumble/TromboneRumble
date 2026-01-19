// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Common/LoadingOverlayWidget.h"
#include "CommonTextBlock.h"

void ULoadingOverlayWidget::Init()
{
	if (CT_Content)
	{
		CT_Content->SetText(FText::FromString(DefaultContent));
	}
}

void ULoadingOverlayWidget::Init(FString InContent)
{
	if (CT_Content)
	{
		CT_Content->SetText(FText::FromString(InContent));
	}
}
