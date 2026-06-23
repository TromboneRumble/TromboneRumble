// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenu/MatchUIRoot.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UMatchUIRoot::PushMenu(const EMatchMenuType InType) const
{
	TSubclassOf<UCommonActivatableWidget> TargetWidgetClass = nullptr;
	switch (InType)
	{
		case EMatchMenuType::MatchMenu:
			TargetWidgetClass = DefaultWidgetClass;
			break;
		default:
			break;
	}
	
	if (TargetWidgetClass)
	{
		BaseStack->AddWidget(TargetWidgetClass);
	}
}