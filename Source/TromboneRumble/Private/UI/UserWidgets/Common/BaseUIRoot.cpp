// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "CommonActivatableWidget.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
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

void UBaseUIRoot::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);

		// Seed focus when the UI is created while a gamepad is already the active input method
		HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
	}
}

void UBaseUIRoot::NativeDestruct()
{
	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
	}

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

void UBaseUIRoot::HandleInputMethodChanged(const ECommonInputType NewInputType)
{
	APlayerController* PC = GetOwningPlayer();

	if (NewInputType == ECommonInputType::Gamepad)
	{
		// Only hide a cursor that is currently shown, so we never fight screens that hide it themselves
		if (PC && PC->ShouldShowMouseCursor())
		{
			PC->SetShowMouseCursor(false);
			bCursorHiddenForGamepad = true;
		}

		FocusActiveWidgetDesiredTarget();
	}
	else if (bCursorHiddenForGamepad)
	{
		bCursorHiddenForGamepad = false;
		if (PC)
		{
			PC->SetShowMouseCursor(true);
		}
	}
}

UCommonActivatableWidget* UBaseUIRoot::GetTopActiveWidget() const
{
	if (PopupStack && PopupStack->GetActiveWidget())
	{
		return PopupStack->GetActiveWidget();
	}
	return BaseStack ? BaseStack->GetActiveWidget() : nullptr;
}

void UBaseUIRoot::FocusActiveWidgetDesiredTarget() const
{
	if (const UCommonActivatableWidget* ActiveWidget = GetTopActiveWidget())
	{
		if (UWidget* FocusTarget = ActiveWidget->GetDesiredFocusTarget())
		{
			FocusTarget->SetFocus();
		}
	}
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

			// Closing a popup doesn't deactivate the base screen, so hand focus back to it for gamepad users
			if (StackType == EUIStackType::Popup)
			{
				const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
				if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
				{
					FocusActiveWidgetDesiredTarget();
				}
			}
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
