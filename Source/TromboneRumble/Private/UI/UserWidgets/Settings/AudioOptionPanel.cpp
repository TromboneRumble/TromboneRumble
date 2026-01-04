// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "CommonTextBlock.h"

void UAudioOptionPanel::Init(TFunction<void()> BackAction)
{
	Super::Init(BackAction);
	
	Text_OptionPanelTitle->SetText(FText::FromString(TEXT("Audio Options")));
}
