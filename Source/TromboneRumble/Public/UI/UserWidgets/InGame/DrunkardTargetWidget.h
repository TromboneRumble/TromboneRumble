// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/PopIndicatorWidget.h"
#include "DrunkardTargetWidget.generated.h"

class UImage;
class UProgressBar;

/** UDrunkardTargetWidget
 *
 * Mark above the drunkard's head that shows who it chases and how long it stays.
 * The fill has the color of the target and drains from the top as the chase time runs out.
 * The icon on top is the face, and the target sees the attack portrait instead while the drunkard is close enough to hit.
 */
UCLASS()
class TROMBONERUMBLE_API UDrunkardTargetWidget : public UPopIndicatorWidget
{
	GENERATED_BODY()

public:

	/** Alpha 0 hides the mark. Any other color tints the fill and pops the mark in, again when the color changes. */
	void SetTargetColor(const FLinearColor& InColor);

	/**
	 * Show the attack portrait when true, and the face when false. The portrait pops in, and the face fades back in.
	 * A hidden mark keeps the icon it has and takes the new one the next time it shows.
	 */
	void SetAttackable(bool bInAttackable);

	/**
	 * Set how much of the fill is left.
	 *
	 * @param InRatio 1 is full and 0 is empty. The fill drains from the top.
	 */
	void SetRemainingRatio(float InRatio);

protected:

	//~ Begin UUserWidget Interface
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget Interface

	//~ Begin UPopIndicatorWidget Interface
	virtual void ApplyShownState() override;
	//~ End UPopIndicatorWidget Interface

	/** Seconds the portrait takes to pop in over the face. */
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Tween", meta = (ClampMin = "0.01"))
	float PortraitPopDuration = 0.2f;

	/** Scale the portrait starts its pop from. */
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Tween", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PortraitPopStartScale = 0.6f;

	/** Seconds the face takes to fade back in over the portrait. */
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Tween", meta = (ClampMin = "0.01"))
	float FaceFadeDuration = 0.1f;

	/**
	 * Part of the fill texture height that is empty below the painted area, 0 to 1.
	 * The bar clips over the whole widget height, so without it the color is gone before the time is.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Fill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FillBottomInset = 0.2f;

	/** Part of the fill texture height that is empty above the painted area, 0 to 1. Without it the color does not shrink at first. */
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Fill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FillTopInset = 0.2f;

private:

	/** Show either the face or the portrait, by bAttackable, at full size and opacity. Ends an icon swap that is still playing. */
	void ApplyIcon();

	/** Set both icons to this point of the swap, 0 at its start and 1 at its end. */
	void ApplyIconSwap(float Alpha);

	/** Set the percent of the bar from RemainingRatio, moved into the painted area of the fill texture. */
	void ApplyFillPercent();

	/** Fill of the target color. Set its Bar Fill Type to Bottom To Top, so a lower percent removes the color from the top. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> FillBar;

	/** Icon everyone sees while the drunkard chases. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Face;

	/** Icon that tells the target to hit the drunkard. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Portrait;

	// The screen space component rebuilds this widget, so the state is kept here and applied again on every construct

	FLinearColor TargetColor = FLinearColor::Transparent;

	float RemainingRatio = 1.f;

	bool bAttackable = false;

	/** Seconds since the icon swap started. Negative while no swap plays. */
	float IconSwapElapsed = -1.f;
};
