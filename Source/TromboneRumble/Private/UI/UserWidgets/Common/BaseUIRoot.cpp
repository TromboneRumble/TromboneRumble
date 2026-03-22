// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "CommonActivatableWidget.h"
#include "UI/UserWidgets/Common/FadeWidget.h"
#include "UI/UserWidgets/Common/LoadingOverlayWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UBaseUIRoot::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (DefaultWidgetClass && UIStack)
	{
		UIStack->AddWidget(DefaultWidgetClass);
	}
}

void UBaseUIRoot::NativeConstruct()
{
	Super::NativeConstruct();
	
	Register();
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
	if (OverlayStack && OverlayStack->GetActiveWidget() && OverlayStack->GetActiveWidget()->IsA<ULoadingOverlayWidget>())
		return;
	
	if (LoadingOverlayWidgetClass)
	{
		OverlayStack->AddWidget(LoadingOverlayWidgetClass);
	}
}

void UBaseUIRoot::PushLoadingOverlay(FString InContent) const
{
	if (OverlayStack && OverlayStack->GetActiveWidget() && OverlayStack->GetActiveWidget()->IsA<ULoadingOverlayWidget>())
		return;
	
	if (LoadingOverlayWidgetClass)
	{
		OverlayStack->AddWidget<ULoadingOverlayWidget>(LoadingOverlayWidgetClass, [this, InContent](ULoadingOverlayWidget& OverlayWidget) 
		{
			OverlayWidget.InitWithContent(InContent);
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

UFadeWidget* UBaseUIRoot::PushFadeOverlay() const
{
	if (OverlayStack && OverlayStack->GetActiveWidget() && OverlayStack->GetActiveWidget()->IsA<UFadeWidget>())
	{
		return Cast<UFadeWidget>(OverlayStack->GetActiveWidget());
	}
	
	if (FadeWidgetClass)
	{
		return OverlayStack->AddWidget<UFadeWidget>(FadeWidgetClass);
	}
	
	return nullptr;
}

void UBaseUIRoot::PopFadeOverlay() const
{
	if (OverlayStack && OverlayStack->GetActiveWidget())
	{
		OverlayStack->GetActiveWidget()->DeactivateWidget();
	}
}

void UBaseUIRoot::Register()
{
}
