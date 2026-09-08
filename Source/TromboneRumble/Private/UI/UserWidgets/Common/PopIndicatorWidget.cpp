// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/PopIndicatorWidget.h"
#include "Subsystems/TweenSubsystem.h"

void UPopIndicatorWidget::SetShown(const bool bInShown)
{
	UTweenSubsystem* TweenSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTweenSubsystem>() : nullptr;

	if (bInShown)
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

void UPopIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyShownState();
}

void UPopIndicatorWidget::HandleHideFinished()
{
	ApplyShownState();
}

void UPopIndicatorWidget::ApplyShownState()
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(bShown ? 1.f : 0.f);
}
