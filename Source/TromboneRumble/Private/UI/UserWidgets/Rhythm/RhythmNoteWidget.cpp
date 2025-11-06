// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmNoteWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void URhythmNoteWidget::InitializeNote(const FVector2D& InStartNorm, const FVector2D& InEndNorm)
{
	StartNorm = InStartNorm;
	EndNorm = InEndNorm;
	UpdatePixelPosition(0.f);
	SetVisibility(ESlateVisibility::Collapsed);
	
}

void URhythmNoteWidget::SetProgress(float InAlpha)
{
	const float Alpha = FMath::Clamp(InAlpha, 0.f, 1.f);
	UpdatePixelPosition(Alpha);
}

void URhythmNoteWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void URhythmNoteWidget::UpdatePixelPosition(float Alpha)
{
	// 정규화 → 픽셀
	const FVector2D ViewSize = UWidgetLayoutLibrary::GetViewportSize(this);
	const FVector2D P = FMath::Lerp(StartNorm, EndNorm, Alpha) * ViewSize;

	if (UPanelSlot* S = Slot)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(S))
		{
			CanvasSlot->SetAnchors(FAnchors(0, 0, 0, 0));          // 좌상단 기준
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));    // 위젯 중심을 좌표에 맞춤
			CanvasSlot->SetPosition(P);                         // 위치만 갱신
			// 크기(Size)는 UMG 디자이너나 SizeBox 등으로 관리
		}
	}
}
