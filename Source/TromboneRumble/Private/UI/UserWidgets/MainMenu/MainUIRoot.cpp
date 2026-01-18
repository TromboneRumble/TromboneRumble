// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UMainUIRoot::PushMenu(const EMainMenuType InType) const
{
	TSubclassOf<UCommonActivatableWidget> TargetWidgetClass = nullptr;
	switch (InType)
	{
		case EMainMenuType::MainMenu:
			TargetWidgetClass = DefaultWidgetClass;
			break;
		case EMainMenuType::Settings:
			TargetWidgetClass = SettingMenuWidgetClass;
			break;
		default:
			break;
	}
	
	if (TargetWidgetClass)
	{
		UIStack->AddWidget(TargetWidgetClass);
	}
}