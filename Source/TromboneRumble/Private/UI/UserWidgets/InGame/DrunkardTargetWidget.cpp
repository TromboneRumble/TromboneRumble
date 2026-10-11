// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/InGame/DrunkardTargetWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Subsystems/TweenSubsystem.h"

void UDrunkardTargetWidget::SetTargetColor(const FLinearColor& InColor)
{
	TargetColor = InColor;
	SetShown(InColor.A > 0.f);
}

void UDrunkardTargetWidget::SetAttackable(const bool bInAttackable)
{
	if (bAttackable == bInAttackable) return;

	bAttackable = bInAttackable;

	// The mark can still be shrinking away, and an icon that changes in that moment looks like a glitch.
	// ApplyShownState takes the new icon when the shrink ends and when the mark shows again
	if (!IsShown()) return;

	IconSwapElapsed = 0.f;
	ApplyIconSwap(0.f);
}

void UDrunkardTargetWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (IconSwapElapsed < 0.f) return;

	IconSwapElapsed += InDeltaTime;

	const float Duration = bAttackable ? PortraitPopDuration : FaceFadeDuration;
	if (IconSwapElapsed >= Duration)
	{
		ApplyIcon();
		return;
	}

	ApplyIconSwap(IconSwapElapsed / Duration);
}

void UDrunkardTargetWidget::SetRemainingRatio(const float InRatio)
{
	RemainingRatio = FMath::Clamp(InRatio, 0.f, 1.f);
	ApplyFillPercent();
}

void UDrunkardTargetWidget::ApplyFillPercent()
{
	if (!FillBar) return;

	// Full is the top of the painted area and empty is its bottom, so the color changes for the whole time and is gone exactly at the end
	const float Bottom = FillBottomInset;
	const float Top = FMath::Max(1.f - FillTopInset, Bottom);
	FillBar->SetPercent(FMath::Lerp(Bottom, Top, RemainingRatio));
}

void UDrunkardTargetWidget::ApplyShownState()
{
	Super::ApplyShownState();

	ApplyFillPercent();

	// The hide tween still shows the last target, so the color changes only while shown
	if (FillBar && IsShown())
	{
		FillBar->SetFillColorAndOpacity(TargetColor);
	}

	ApplyIcon();
}

void UDrunkardTargetWidget::ApplyIcon()
{
	IconSwapElapsed = -1.f;

	if (Face)
	{
		Face->SetVisibility(bAttackable ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		Face->SetRenderOpacity(1.f);
	}
	if (Portrait)
	{
		Portrait->SetVisibility(bAttackable ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		Portrait->SetRenderOpacity(1.f);
		Portrait->SetRenderScale(FVector2D::UnitVector);
	}
}

void UDrunkardTargetWidget::ApplyIconSwap(const float Alpha)
{
	if (!Face || !Portrait) return;

	if (bAttackable)
	{
		// The portrait tells the target to hit now, so it jumps out. UTweenSubsystem scales only a whole user widget, so only its curve is used here
		const float Scale = FMath::Lerp(PortraitPopStartScale, 1.f, UTweenSubsystem::EaseOutBack(Alpha));

		Face->SetVisibility(ESlateVisibility::Collapsed);
		Portrait->SetVisibility(ESlateVisibility::HitTestInvisible);
		Portrait->SetRenderOpacity(1.f);
		Portrait->SetRenderScale(FVector2D(Scale));
	}
	else
	{
		// Going back to the face only ends that signal, so it is a short and quiet cross fade
		Face->SetVisibility(ESlateVisibility::HitTestInvisible);
		Face->SetRenderOpacity(Alpha);
		Portrait->SetVisibility(ESlateVisibility::HitTestInvisible);
		Portrait->SetRenderOpacity(1.f - Alpha);
		Portrait->SetRenderScale(FVector2D::UnitVector);
	}
}
