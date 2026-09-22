// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickTimeline.h"
#include "Widgets/SLeafWidget.h"

/** One row of SGimmickTimeline. */
struct FGimmickTimelineRow
{
	/** Name of the gimmick, drawn left of the row. */
	FText Label;

	/** Limit of the prediction, shown when the mouse is over the label. Can be empty. */
	FText Note;

	TArray<FGimmickTimelineSpan> Spans;

	/** Does fever time change this gimmick. The row then shows where fever time starts. */
	bool bUsesFever = false;
};

/**
 * SGimmickTimeline draws when each gimmick of a level is expected to run during one round.
 * One row per gimmick, time from left to right, one colored block per phase.
 * SGimmickSettingsPanel owns it and fills it from UGimmickConfig::BuildTimeline.
 *
 * Keep in mind that it only draws the rows it is given. It reads no config by itself.
 *
 * @see SGimmickSettingsPanel
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

	float RoundLength = 210.f;

	float FeverStart = 168.f;

	/** Tooltip of the block or label under the mouse. */
	FText HoverText;
};
