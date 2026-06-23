// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "CommonActivatableWidget.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UBaseUIRoot::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (DefaultWidgetClass && BaseStack)
	{
		BaseStack->AddWidget(DefaultWidgetClass);
	}
}

void UBaseUIRoot::NativeDestruct()
{
	if (BaseStack)
	{
		BaseStack->ClearWidgets();
	}
	if (PopupStack)
	{
		PopupStack->ClearWidgets();
	}
	if (OverlayStack)
	{
		OverlayStack->ClearWidgets();
	}
	
	Super::NativeDestruct();
}

UCommonActivatableWidget* UBaseUIRoot::AddWidgetToStack(const TSubclassOf<UCommonActivatableWidget> WidgetClass, const EUIStackType StackType) const
{
	if (!WidgetClass)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Invalid WidgetClass."));
		return nullptr;
	}

	UCommonActivatableWidgetStack* TargetStack = GetStackByType(StackType);
	if (!TargetStack)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Invalid StackType."));
		return nullptr;
	}

	if (const UCommonActivatableWidget* ActiveWidget = TargetStack->GetActiveWidget())
	{
		if (ActiveWidget->GetClass() == WidgetClass)
		{
			return TargetStack->GetActiveWidget();
		}
	}

	return TargetStack->AddWidget(WidgetClass);
}

bool UBaseUIRoot::PopStack(const EUIStackType StackType) const
{
	if (const UCommonActivatableWidgetStack* TargetStack = GetStackByType(StackType))
	{
		if (UCommonActivatableWidget* ActiveWidget = TargetStack->GetActiveWidget())
		{
			ActiveWidget->DeactivateWidget();
			return true;
		}
	}
	return false;
}

void UBaseUIRoot::SetBaseUIEnabled(const bool bEnabled) const
{
	if (BaseStack)
	{
		if (UCommonActivatableWidget* ActiveWidget = BaseStack->GetActiveWidget())
		{
			if (UBaseMenuWidget* MenuWidget = Cast<UBaseMenuWidget>(ActiveWidget))
			{
				MenuWidget->SetUIEnabled(bEnabled);
			}
		}
	}
}

UCommonActivatableWidgetStack* UBaseUIRoot::GetStackByType(const EUIStackType StackType) const
{
	switch (StackType)
	{
	case EUIStackType::Base:
		return BaseStack;
	case EUIStackType::Popup:
		return PopupStack;
	case EUIStackType::Overlay:
		return OverlayStack;
	default:
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Invalid StackType."));
		return nullptr;
	}
}
