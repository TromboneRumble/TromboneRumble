// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/VideoOptionPanel.h"
#include "CommonTextBlock.h"

void UVideoOptionPanel::Init(TFunction<void()> BackAction)
{
	Super::Init(BackAction);
	
	Text_OptionPanelTitle->SetText(FText::FromString(TEXT("Video Options")));
}
