// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/UserWidgets/Rhythm/RhythmNoteWidget.h"
#include "UI/UserWidgets/Rhythm/RhythmSpawnWidget.h"
#include "Utilities/DebugHelper.h"

URhythmNoteWidget* URhythmUIRootWidget::SpawnNote(int32 LaneIndex)
{
	if (!RhythmSpawnWidget || !NoteWidgetClass) return nullptr;

	URhythmNoteWidget* Note = CreateWidget<URhythmNoteWidget>(this, NoteWidgetClass);
	if (!Note) return nullptr;

	UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(NoteCanvas->AddChild(Note));
	if (!NoteSlot) return Note;

	SetStartXPos();
	for (int32 i = 0; i < MaxLanes; ++i)
	{
		SetStartYPos(i);
	}

	FVector2D StartPos(LaneXStartPos, GetLaneY(LaneIndex));
	FVector2D EndPos(LaneXEndPos, GetLaneY(LaneIndex));
	NoteSlot->SetAnchors(FAnchors(0.f, 0.f));
	NoteSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	NoteSlot->SetAutoSize(true);
	NoteSlot->SetPosition(StartPos);
	UE_LOG(LogTemp, Warning, TEXT("StartPos : %f, %f"), StartPos.X, StartPos.Y);
	return Note;
}



void URhythmUIRootWidget::UpdateNoteProgress(URhythmNoteWidget* Note, float Alpha01)
{
	if (!Note) return;
	Note->SetProgress(Alpha01);
}

void URhythmUIRootWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &ThisClass::OnViewPortResizedHandler);
	}
	LaneYPosArray.SetNum(MaxLanes);
}

void URhythmUIRootWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	/*if (!bInitializedPositions && RhythmSpawnWidget)
	{
		SetStartXPos();

		for (int32 i = 0; i < MaxLanes; ++i)
			SetStartYPos(i);

		bInitializedPositions = true;
	}*/
}

void URhythmUIRootWidget::OnViewPortResizedHandler(FViewport* ViewPort, uint32)
{
	FIntPoint Size = ViewPort->GetSizeXY();
	SetStartXPos();
	for (int32 i = 0; i < MaxLanes; ++i)
	{
		SetStartYPos(i);
	}
}

void URhythmUIRootWidget::SetStartXPos()
{
	checkf(RhythmSpawnWidget, TEXT("RhythmSpawnWidget is nullptr"));
	checkf(NoteCanvas, TEXT("NoteCanvas is nullptr"));

	const FGeometry CanvasGeo = NoteCanvas->GetCachedGeometry();
	const FGeometry ChildGeo = RhythmSpawnWidget->GetCachedGeometry();

	// SpawnWidget의 절대 좌표계
	const FVector2D ChildAbsPos = ChildGeo.GetAbsolutePosition();
	const FVector2D ChildAbsSize = ChildGeo.GetAbsoluteSize();

	// 절대 좌표 -> 로컬 좌표
	const FVector2D TL_Local = CanvasGeo.AbsoluteToLocal(ChildAbsPos);
	const FVector2D TR_Local = CanvasGeo.AbsoluteToLocal(ChildAbsPos + FVector2D(ChildAbsSize.X, 0.f));

	LaneXStartPos = TR_Local.X;
	LaneXEndPos = TL_Local.X;
}

void URhythmUIRootWidget::SetStartYPos(int32 LaneIndex)
{
	checkf(RhythmSpawnWidget, TEXT("RhythmSpawnWidget is nullptr"));
	checkf(NoteCanvas, TEXT("NoteCanvas is nullptr"));

	const FGeometry CanvasGeo = NoteCanvas->GetCachedGeometry();
	const FGeometry ChildGeo = RhythmSpawnWidget->GetCachedGeometry();

	// SpawnWidget의 절대 좌표계
	const FVector2D ChildAbsPos = ChildGeo.GetAbsolutePosition();
	const FVector2D ChildAbsSize = ChildGeo.GetAbsoluteSize();

	// 절대 좌표 -> NoteCanvas 로컬 좌표
	const FVector2D TL_Local = CanvasGeo.AbsoluteToLocal(ChildAbsPos);
	const FVector2D BL_Local = CanvasGeo.AbsoluteToLocal(ChildAbsPos + FVector2D(0.f, ChildAbsSize.Y));

	const float TopY_Local = TL_Local.Y;
	const float Height_Local = BL_Local.Y - TL_Local.Y;

	const float Step = Height_Local / static_cast<float>(MaxLanes + 1);

	// 등분선: 위에서 (LaneIndex+1)칸 내려온 선을 lane 센터로 사용
	const float LaneCenterY = TopY_Local + Step * static_cast<float>(LaneIndex + 1);
	
	if (LaneYPosArray.IsValidIndex(LaneIndex))
	{
		LaneYPosArray[LaneIndex] = LaneCenterY;
		UE_LOG(LogTemp, Warning, TEXT("SetStartYPos : %f"), LaneCenterY);
	}

}

float URhythmUIRootWidget::GetLaneY(int32 LaneIndex) const
{
	if (LaneYPosArray.IsValidIndex(LaneIndex))
	{
		return LaneYPosArray[LaneIndex];
	}
	return 0;
}
