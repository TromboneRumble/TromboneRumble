// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/RootUI.h"
#include "CommonActivatableWidget.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "UI/UserWidgets/Common/ProjectVersionWidget.h"
#include "UI/UserWidgets/InGame/SubWidgets/PerformanceWidget.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void URootUI::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	// Designer preview only. At runtime the HUD of each level pushes its own screen
	if (IsDesignTime() && DefaultWidgetClass)
	{
		AddWidgetToStack(DefaultWidgetClass, EUIStackType::Base);
	}
}

void URootUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);

		// Seed focus when the UI is created while a gamepad is already the active input method
		HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
	}
}

void URootUI::NativeDestruct()
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

void URootUI::HandleInputMethodChanged(const ECommonInputType NewInputType)
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

UCommonActivatableWidget* URootUI::GetTopActiveWidget() const
{
	// Overlay sits above Popup, and Popup above Base. Ask them in that order
	if (OverlayStack && OverlayStack->GetActiveWidget())
	{
		return OverlayStack->GetActiveWidget();
	}
	if (PopupStack && PopupStack->GetActiveWidget())
	{
		return PopupStack->GetActiveWidget();
	}
	return BaseStack ? BaseStack->GetActiveWidget() : nullptr;
}

void URootUI::SetPerformanceWidgetVisible(const bool bVisible) const
{
	if (PerformanceWidget)
	{
		PerformanceWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void URootUI::FocusActiveWidgetDesiredTarget() const
{
	if (const UCommonActivatableWidget* ActiveWidget = GetTopActiveWidget())
	{
		if (UWidget* FocusTarget = ActiveWidget->GetDesiredFocusTarget())
		{
			FocusTarget->SetFocus();
		}
	}
}

UCommonActivatableWidget* URootUI::AddWidgetToStack(const TSubclassOf<UCommonActivatableWidget> WidgetClass, const EUIStackType StackType) const
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

bool URootUI::PopStack(const EUIStackType StackType) const
{
	if (const UCommonActivatableWidgetStack* TargetStack = GetStackByType(StackType))
	{
		if (UCommonActivatableWidget* ActiveWidget = TargetStack->GetActiveWidget())
		{
			ActiveWidget->DeactivateWidget();

			// Closing a layer never reactivates the screen below, so hand focus back to it for gamepad users
			if (StackType != EUIStackType::Base)
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

UCommonActivatableWidgetStack* URootUI::GetStackByType(const EUIStackType StackType) const
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
