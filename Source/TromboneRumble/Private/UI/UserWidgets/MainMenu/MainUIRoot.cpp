// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UMainUIRoot::NativePreConstruct()
{
	Super::NativeConstruct();
	
	if (MainMenuWidgetClass && MenuStack)
	{
		MenuStack->AddWidget(MainMenuWidgetClass);
	}
}

void UMainUIRoot::NativeDestruct()
{
	if (MenuStack)
	{
		MenuStack->ClearWidgets();
	}
	
	Super::NativeDestruct();
}

void UMainUIRoot::PushMenu(const EMainMenuType InType) const
{
	TSubclassOf<UCommonActivatableWidget> TargetWidgetClass = nullptr;
	switch (InType)
	{
		case EMainMenuType::MainMenu:
			TargetWidgetClass = MainMenuWidgetClass;
			break;
		case EMainMenuType::Settings:
			TargetWidgetClass = SettingMenuWidgetClass;
			break;
		default:
			break;
	}
	
	if (TargetWidgetClass)
	{
		MenuStack->AddWidget(TargetWidgetClass);
	}
}