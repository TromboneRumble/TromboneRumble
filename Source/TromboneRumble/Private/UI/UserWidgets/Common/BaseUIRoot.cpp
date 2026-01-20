// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "CommonActivatableWidget.h"
#include "UI/UserWidgets/Common/LoadingOverlayWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

class ULoadingOverlayWidget;

void UBaseUIRoot::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (DefaultWidgetClass && UIStack)
	{
		UIStack->AddWidget(DefaultWidgetClass);
	}
}

void UBaseUIRoot::NativeDestruct()
{
	if (UIStack)
	{
		UIStack->ClearWidgets();
	}
	
	Super::NativeDestruct();
}

void UBaseUIRoot::PushLoadingOverlay() const
{
	if (LoadingOverlayWidgetClass)
	{
		OverlayStack->AddWidget(LoadingOverlayWidgetClass);
	}
}

void UBaseUIRoot::PushLoadingOverlay(FString InContent) const
{
	if (LoadingOverlayWidgetClass)
	{
		OverlayStack->AddWidget<ULoadingOverlayWidget>(LoadingOverlayWidgetClass, [this, InContent](ULoadingOverlayWidget& OverlayWidget) 
		{
			OverlayWidget.Init(InContent);
		});
	}
}

void UBaseUIRoot::PopLoadingOverlay() const
{
	if (OverlayStack && OverlayStack->GetActiveWidget())
	{
		OverlayStack->GetActiveWidget()->DeactivateWidget();
	}
}
