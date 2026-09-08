// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/InGame/DrunkardTargetWidget.h"
#include "Components/Image.h"
#include "Subsystems/TweenSubsystem.h"

void UDrunkardTargetWidget::SetTargetColor(const FLinearColor& InColor)
{
	const bool bNewShown = InColor.A > 0.f;
	TargetColor = InColor;

	UTweenSubsystem* TweenSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTweenSubsystem>() : nullptr;

	if (bNewShown)
	{
		bShown = true;
		ApplyShownState();
		
		if (TweenSubsystem)
		{
			SetRenderScale(FVector2D::ZeroVector);
			TweenSubsystem->DoTween(this, FVector2D::UnitVector, AppearDuration, ETweenCurveType::EaseOutBack);
		}
		return;
	}

	if (!bShown)
	{
		return;
	}
	bShown = false;

	if (TweenSubsystem)
	{
		TweenSubsystem->DoTween(this, FVector2D::ZeroVector, HideDuration, ETweenCurveType::EaseIn,
			FSimpleDelegate::CreateUObject(this, &ThisClass::HandleHideFinished));
	}
	else
	{
		ApplyShownState();
	}
}

void UDrunkardTargetWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyShownState();
}

void UDrunkardTargetWidget::HandleHideFinished()
{
	ApplyShownState();
}

void UDrunkardTargetWidget::ApplyShownState()
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(bShown ? 1.f : 0.f);

	if (PortraitBackground && bShown)
	{
		PortraitBackground->SetColorAndOpacity(TargetColor);
	}
}
