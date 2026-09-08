// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/InGame/DrunkardAttackableWidget.h"
#include "CommonActionWidget.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "Components/Image.h"

void UDrunkardAttackableWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AttackActionGlyph && !AttackActionRow.IsNull())
	{
		AttackActionGlyph->SetInputAction(AttackActionRow);
	}
}

void UDrunkardAttackableWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
		HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
	}
}

void UDrunkardAttackableWidget::NativeDestruct()
{
	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDrunkardAttackableWidget::HandleInputMethodChanged(const ECommonInputType NewInputType)
{
	const bool bGlyph = AttackActionGlyph && NewInputType == ECommonInputType::Gamepad;

	if (AttackActionGlyph)
	{
		AttackActionGlyph->SetRenderOpacity(bGlyph ? 1.f : 0.f);
	}
	if (Icon)
	{
		Icon->SetVisibility(bGlyph ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}
