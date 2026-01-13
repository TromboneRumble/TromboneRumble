// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenu/NoticePopupWidget.h"
#include "CommonTextBlock.h"

void UNoticePopupWidget::OnInit(const FString InContent)
{
	Init();
	
	if (Text_Content)
	{
		Text_Content->SetText(FText::FromString(InContent));
	}
}

void UNoticePopupWidget::OnInit(const FString InTitle, const FString InContent)
{
	Init();
	
	if (Text_Title)
	{
		Text_Title->SetText(FText::FromString(InTitle));
	}
	if (Text_Content)
	{
		Text_Content->SetText(FText::FromString(InContent));
	}
}