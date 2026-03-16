// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Common/LoadingOverlayWidget.h"
#include "CommonTextBlock.h"

void ULoadingOverlayWidget::InitDefault()
{
	if (CT_Content)
	{
		CT_Content->SetText(FText::FromString(DefaultContent));
	}
}

void ULoadingOverlayWidget::InitWithContent(const FString& InContent)
{
	FString FinalContent = InContent.IsEmpty() ? DefaultContent : InContent;
	
	if (CT_Content)
	{
		CT_Content->SetText(FText::FromString(FinalContent));
	}
}