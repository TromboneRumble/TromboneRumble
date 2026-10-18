// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickTimeline.h"
#include "Widgets/SLeafWidget.h"

/**
 * SGimmickTimeline draws when each gimmick of a level is expected to run during one round.
 * One row per gimmick, time from left to right, one colored block per phase.
 * SGimmickTimelineTab owns it and fills it from UStageGimmickData::BuildTimeline.
 *
 * Keep in mind that it only draws the rows it is given. It reads no config by itself.
 *
 * @see SGimmickTimelineTab
 * @see FGimmickTimelineBuilder
 */
class SGimmickTimeline : public SLeafWidget
{
public:

	SLATE_BEGIN_ARGS(SGimmickTimeline) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/**
	 * Replace everything the widget draws.
	 *
	 * @param InRows One row per gimmick.
	 * @param InRoundLength Seconds the time axis covers.
	 * @param InFeverStart Second fever time starts at. Drawn only on rows that use fever time.
	 */
	void SetRows(TArray<FGimmickTimelineRow> InRows, float InRoundLength, float InFeverStart);

	/** @return Color of the blocks of this phase. The panel uses it for the legend. */
	static FLinearColor GetPhaseColor(EGimmickTimelinePhase Phase);

	/** @return Seconds as minutes and seconds, such as "1:35" for 95. The time axis and the song list of the timeline tab use it. */
	static FText FormatTime(float Seconds);

	//~ Begin SWidget Interface
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	//~ End SWidget Interface

private:

	/** @return Tooltip for this position in the widget. Empty when the mouse is over nothing. */
	FText FindHoverText(const FVector2f& LocalPosition, float Width) const;

	TArray<FGimmickTimelineRow> Rows;

	/** Seconds the time axis covers. Set by SetRows. Never 0, because the width is divided by it. */
	float RoundLength = 1.f;

	/** Second fever time starts at. Set by SetRows. */
	float FeverStart = 0.f;

	/** Tooltip of the block or label under the mouse. */
	FText HoverText;
};
