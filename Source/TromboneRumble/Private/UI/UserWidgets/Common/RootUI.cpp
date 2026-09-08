// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/RootUI.h"
#include "CommonActionWidget.h"
#include "CommonActivatableWidget.h"
#include "CommonInputSettings.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "ICommonInputModule.h"
#include "TromboneGamePlayTags.h"
#include "UI/UserWidgets/InGame/SubWidgets/PerformanceWidget.h"
#include "Utilities/DebugHelper.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void URootUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterDefaultLayers();

	// The accept glyph is the engine's default click action, so it swaps between A and Enter by itself
	if (AcceptActionWidget)
	{
		const UCommonInputSettings& InputSettings = ICommonInputModule::GetSettings();
		if (UInputAction* EnhancedClickAction = UCommonInputSettings::IsEnhancedInputSupportEnabled() ? InputSettings.GetEnhancedInputClickAction() : nullptr)
		{
			AcceptActionWidget->SetEnhancedInputAction(EnhancedClickAction);
		}
		else
		{
			AcceptActionWidget->SetInputAction(InputSettings.GetDefaultClickAction());
		}
	}
}

void URootUI::NativePreConstruct()
{
	Super::NativePreConstruct();

	// The designer preview does not always go through NativeOnInitialized
	if (Layers.IsEmpty())
	{
		RegisterDefaultLayers();
	}

	// Designer preview only. At runtime the HUD of each level pushes its own screen, and the class is never loaded
	if (IsDesignTime() && !DefaultWidgetClass.IsNull())
	{
		AddWidgetToStack(DefaultWidgetClass.LoadSynchronous(), TromboneGamePlayTags::Trombone_UI_Layer_Base);
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

	for (const TPair<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>>& Layer : Layers)
	{
		if (Layer.Value)
		{
			Layer.Value->ClearWidgets();
		}
	}

	Super::NativeDestruct();
}

void URootUI::RegisterDefaultLayers()
{
	// Bottom to top. The order decides focus priority
	RegisterLayer(TromboneGamePlayTags::Trombone_UI_Layer_Base, BaseStack);
	RegisterLayer(TromboneGamePlayTags::Trombone_UI_Layer_Popup, PopupStack);
	RegisterLayer(TromboneGamePlayTags::Trombone_UI_Layer_Overlay, OverlayStack);
}

void URootUI::RegisterLayer(const FGameplayTag LayerTag, UCommonActivatableWidgetStack* Stack)
{
	if (!LayerTag.IsValid() || !Stack)
	{
		return;
	}

	if (!Layers.Contains(LayerTag))
	{
		LayerOrder.Add(LayerTag);
	}
	Layers.Add(LayerTag, Stack);

	// Registration can run twice for the same stack (initialize and designer preview), so rebind instead of adding
	Stack->OnDisplayedWidgetChanged().RemoveAll(this);
	Stack->OnDisplayedWidgetChanged().AddUObject(this, &ThisClass::HandleDisplayedWidgetChanged);
}

void URootUI::HandleDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget) const
{
	UpdateActionBar();
}

void URootUI::UpdateActionBar() const
{
	if (!ActionBar)
	{
		return;
	}

	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	const bool bGamepad = InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
	ActionBar->SetVisibility(bGamepad ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	// Nothing to press when the active screen has no focus target, like the in game HUD
	if (AcceptAction)
	{
		const UCommonActivatableWidget* ActiveWidget = GetTopActiveWidget();
		const bool bCanAccept = ActiveWidget && ActiveWidget->GetDesiredFocusTarget() != nullptr;
		AcceptAction->SetVisibility(bCanAccept ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

UCommonActivatableWidgetStack* URootUI::GetLayer(const FGameplayTag LayerTag) const
{
	UCommonActivatableWidgetStack* Stack = Layers.FindRef(LayerTag);
	if (!Stack)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("No layer registered for the tag."));
	}
	return Stack;
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

	UpdateActionBar();
}

UCommonActivatableWidget* URootUI::GetTopActiveWidget() const
{
	// Walk from the top most layer down. The first active widget owns focus
	for (int32 Index = LayerOrder.Num() - 1; Index >= 0; --Index)
	{
		if (const UCommonActivatableWidgetStack* Stack = Layers.FindRef(LayerOrder[Index]))
		{
			if (UCommonActivatableWidget* ActiveWidget = Stack->GetActiveWidget())
			{
				return ActiveWidget;
			}
		}
	}
	return nullptr;
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

FName URootUI::SuspendInput(const FName Reason)
{
	UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	if (!InputSubsystem)
	{
		return NAME_None;
	}

	// Each call gets its own token so overlapping loads do not release each other's block
	FName Token = Reason;
	Token.SetNumber(++InputSuspensions);

	InputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, Token, true);
	InputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, Token, true);
	InputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, Token, true);
	return Token;
}

void URootUI::ResumeInput(const FName Token)
{
	if (Token == NAME_None)
	{
		return;
	}

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, Token, false);
		InputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, Token, false);
		InputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, Token, false);
	}
}

UCommonActivatableWidget* URootUI::AddWidgetToStack(const TSubclassOf<UCommonActivatableWidget> WidgetClass, const FGameplayTag LayerTag) const
{
	if (!WidgetClass)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Invalid WidgetClass."));
		return nullptr;
	}

	UCommonActivatableWidgetStack* TargetStack = GetLayer(LayerTag);
	if (!TargetStack)
	{
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

bool URootUI::PopStack(const FGameplayTag LayerTag) const
{
	const UCommonActivatableWidgetStack* TargetStack = GetLayer(LayerTag);
	if (!TargetStack)
	{
		return false;
	}

	UCommonActivatableWidget* ActiveWidget = TargetStack->GetActiveWidget();
	if (!ActiveWidget)
	{
		return false;
	}

	ActiveWidget->DeactivateWidget();

	// Closing a layer never reactivates the screen below, so hand focus back to it for gamepad users
	const bool bIsBottomLayer = LayerOrder.Num() > 0 && LayerOrder[0] == LayerTag;
	if (!bIsBottomLayer)
	{
		const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
		if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
		{
			FocusActiveWidgetDesiredTarget();
		}
	}
	return true;
}
