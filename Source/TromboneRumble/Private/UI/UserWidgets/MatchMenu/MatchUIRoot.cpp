// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenu/MatchUIRoot.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UMatchUIRoot::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (MatchMenuWidgetClass && MenuStack)
	{
		MenuStack->AddWidget(MatchMenuWidgetClass);
	}
}

void UMatchUIRoot::NativeDestruct()
{
	if (MenuStack)
	{
		MenuStack->ClearWidgets();
	}
	
	Super::NativeDestruct();
}

void UMatchUIRoot::PushMenu(const EMatchMenuType InType) const
{
	TSubclassOf<UCommonActivatableWidget> TargetWidgetClass = nullptr;
	switch (InType)
	{
		case EMatchMenuType::MatchMenu:
			TargetWidgetClass = MatchMenuWidgetClass;
			break;
		default:
			break;
	}
	
	if (TargetWidgetClass)
	{
		MenuStack->AddWidget(TargetWidgetClass);
	}
}