// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "CommonTextBlock.h"

void UNoticePopupWidget::OnInit(const FText& InContent)
{
	Init();
	
	if (Text_Content)
	{
		Text_Content->SetText(InContent);
	}
}

void UNoticePopupWidget::OnInit(const FText& InTitle, const FText& InContent)
{
	Init();
	
	if (Text_Title)
	{
		Text_Title->SetText(InTitle);
	}
	if (Text_Content)
	{
		Text_Content->SetText(InContent);
	}
}