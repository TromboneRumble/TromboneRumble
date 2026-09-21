// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/SGimmickTimeline.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"

namespace
{
	constexpr float LabelWidth = 130.f;
	constexpr float RightPadding = 12.f;
	constexpr float AxisHeight = 22.f;
	constexpr float RowHeight = 22.f;
	constexpr float RowGap = 6.f;

	/** A spawn has no length and a short phase can be below one pixel, so every block is at least this wide. */
	constexpr float MinBlockWidth = 2.f;

	constexpr float MinorTickSeconds = 10.f;
	constexpr float MajorTickSeconds = 30.f;

	float GetRowTop(const int32 RowIndex)
	{
		return AxisHeight + RowIndex * (RowHeight + RowGap);
	}

	/** 95 seconds becomes "1:35". */
	FText FormatTime(const float Seconds)
	{
		const int32 Total = FMath::RoundToInt32(Seconds);
		return FText::FromString(FString::Printf(TEXT("%d:%02d"), Total / 60, Total % 60));
	}

	void DrawBox(FSlateWindowElementList& OutDrawElements, const int32 LayerId, const FGeometry& Geometry, const FVector2f& Position, const FVector2f& Size, const FLinearColor& Color)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			Geometry.ToPaintGeometry(Size, FSlateLayoutTransform(Position)),
			FAppStyle::GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			Color);
	}

	void DrawText(FSlateWindowElementList& OutDrawElements, const int32 LayerId, const FGeometry& Geometry, const FVector2f& Position, const FText& Text, const FLinearColor& Color)
	{
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			Geometry.ToPaintGeometry(FVector2f(LabelWidth, RowHeight), FSlateLayoutTransform(Position)),
			Text,
			FAppStyle::GetFontStyle("SmallFont"),
			ESlateDrawEffect::None,
			Color);
	}
}

void SGimmickTimeline::Construct(const FArguments& InArgs)
{
	SetToolTipText(TAttribute<FText>::CreateLambda([this]() { return HoverText; }));
}

void SGimmickTimeline::SetRows(TArray<FGimmickTimelineRow> InRows, const float InRoundLength, const float InFeverStart)
{
	Rows = MoveTemp(InRows);
	RoundLength = FMath::Max(1.f, InRoundLength);
	FeverStart = InFeverStart;
	HoverText = FText::GetEmpty();

	// The row count decides the height
	Invalidate(EInvalidateWidgetReason::Layout);
}

FLinearColor SGimmickTimeline::GetPhaseColor(const EGimmickTimelinePhase Phase)
{
	switch (Phase)
	{
	case EGimmickTimelinePhase::Warning: return FLinearColor(0.95f, 0.70f, 0.10f);
	case EGimmickTimelinePhase::Active:  return FLinearColor(0.85f, 0.22f, 0.18f);
	case EGimmickTimelinePhase::Ending:  return FLinearColor(0.30f, 0.50f, 0.75f);
	case EGimmickTimelinePhase::Spawn:   return FLinearColor(0.85f, 0.85f, 0.85f);
	default:                             return FLinearColor::White;
	}
}

FVector2D SGimmickTimeline::ComputeDesiredSize(const float LayoutScaleMultiplier) const
{
	return FVector2D(480.0, GetRowTop(Rows.Num()));
}

int32 SGimmickTimeline::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, const bool bParentEnabled) const
{
	const float Width = AllottedGeometry.GetLocalSize().X;
	const float TrackWidth = FMath::Max(1.f, Width - LabelWidth - RightPadding);
	const float PixelsPerSecond = TrackWidth / RoundLength;
	const float Bottom = GetRowTop(Rows.Num());

	const FLinearColor TextColor = InWidgetStyle.GetForegroundColor();
	const FLinearColor RowColor(1.f, 1.f, 1.f, 0.04f);
	const FLinearColor MinorTickColor(1.f, 1.f, 1.f, 0.06f);
	const FLinearColor MajorTickColor(1.f, 1.f, 1.f, 0.18f);
	const FLinearColor FeverColor(0.80f, 0.35f, 0.90f, 0.16f);

	const int32 BackgroundLayer = LayerId;
	const int32 GridLayer = LayerId + 1;
	const int32 BlockLayer = LayerId + 2;

	// Row backgrounds, fever time and labels
	for (int32 RowIndex = 0; RowIndex < Rows.Num(); ++RowIndex)
	{
		const FGimmickTimelineRow& Row = Rows[RowIndex];
		const float Top = GetRowTop(RowIndex);

		DrawBox(OutDrawElements, BackgroundLayer, AllottedGeometry, FVector2f(LabelWidth, Top), FVector2f(TrackWidth, RowHeight), RowColor);

		if (Row.bUsesFever && FeverStart < RoundLength)
		{
			const float FeverX = LabelWidth + FMath::Max(0.f, FeverStart) * PixelsPerSecond;
			DrawBox(OutDrawElements, BackgroundLayer, AllottedGeometry, FVector2f(FeverX, Top), FVector2f(LabelWidth + TrackWidth - FeverX, RowHeight), FeverColor);
		}

		DrawText(OutDrawElements, BlockLayer, AllottedGeometry, FVector2f(0.f, Top + 4.f), Row.Label, TextColor);
	}

	// Time axis. A line every 10 seconds, a stronger line and the time every 30 seconds
	for (float Time = 0.f; Time <= RoundLength; Time += MinorTickSeconds)
	{
		const bool bMajor = FMath::IsNearlyZero(FMath::Fmod(Time, MajorTickSeconds));
		const float X = LabelWidth + Time * PixelsPerSecond;

		DrawBox(OutDrawElements, GridLayer, AllottedGeometry, FVector2f(X, AxisHeight - 4.f), FVector2f(1.f, Bottom - AxisHeight + 4.f), bMajor ? MajorTickColor : MinorTickColor);

		if (bMajor)
		{
			DrawText(OutDrawElements, GridLayer, AllottedGeometry, FVector2f(X + 3.f, 2.f), FormatTime(Time), TextColor.CopyWithNewOpacity(0.6f));
		}
	}

	// Blocks
	for (int32 RowIndex = 0; RowIndex < Rows.Num(); ++RowIndex)
	{
		const float Top = GetRowTop(RowIndex);

		for (const FGimmickTimelineSpan& Span : Rows[RowIndex].Spans)
		{
			const float End = FMath::Min(Span.Start + Span.Duration, RoundLength);
			const float X = LabelWidth + Span.Start * PixelsPerSecond;
			const float BlockWidth = FMath::Max(MinBlockWidth, (End - Span.Start) * PixelsPerSecond);

			DrawBox(OutDrawElements, BlockLayer, AllottedGeometry, FVector2f(X, Top + 2.f), FVector2f(BlockWidth, RowHeight - 4.f), GetPhaseColor(Span.Phase));
		}
	}

	return BlockLayer;
}

FReply SGimmickTimeline::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2f LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	HoverText = FindHoverText(LocalPosition, MyGeometry.GetLocalSize().X);

	return FReply::Unhandled();
}

void SGimmickTimeline::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	HoverText = FText::GetEmpty();
}

FText SGimmickTimeline::FindHoverText(const FVector2f& LocalPosition, const float Width) const
{
	if (LocalPosition.Y < AxisHeight) return FText::GetEmpty();

	const int32 RowIndex = FMath::FloorToInt32((LocalPosition.Y - AxisHeight) / (RowHeight + RowGap));
	if (!Rows.IsValidIndex(RowIndex)) return FText::GetEmpty();

	const FGimmickTimelineRow& Row = Rows[RowIndex];
	if (LocalPosition.X < LabelWidth) return Row.Note;

	const float TrackWidth = FMath::Max(1.f, Width - LabelWidth - RightPadding);
	const float PixelsPerSecond = TrackWidth / RoundLength;

	// Blocks of one row can overlap, such as spotlight zones in fever time. The last one is drawn on top, so it wins
	for (int32 SpanIndex = Row.Spans.Num() - 1; SpanIndex >= 0; --SpanIndex)
	{
		const FGimmickTimelineSpan& Span = Row.Spans[SpanIndex];
		const float X = LabelWidth + Span.Start * PixelsPerSecond;
		const float BlockWidth = FMath::Max(MinBlockWidth, Span.Duration * PixelsPerSecond);

		if (LocalPosition.X < X || LocalPosition.X > X + BlockWidth) continue;

		if (Span.Phase == EGimmickTimelinePhase::Spawn)
		{
			return FText::Format(INVTEXT("{0}: {1}  {2}"), Row.Label, Span.Label, FormatTime(Span.Start));
		}

		return FText::Format(INVTEXT("{0}: {1}  {2} ~ {3}  ({4}초)"),
			Row.Label, Span.Label, FormatTime(Span.Start), FormatTime(Span.Start + Span.Duration), FText::AsNumber(Span.Duration));
	}

	return FText::GetEmpty();
}
