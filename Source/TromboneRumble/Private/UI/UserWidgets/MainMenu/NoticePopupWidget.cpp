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