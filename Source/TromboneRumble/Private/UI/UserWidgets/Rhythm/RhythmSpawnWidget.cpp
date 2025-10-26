// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmSpawnWidget.h"
#include "UI/UserWidgets/Rhythm/RhythmNoteWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Utilities/DebugHelper.h"


URhythmNoteWidget* URhythmSpawnWidget::SpawnNote(int32 LaneIndex)
{
	if (!NoteWidgetClass) return nullptr;

	URhythmNoteWidget* Note = CreateWidget<URhythmNoteWidget>(this, NoteWidgetClass);
	if (!Note) return nullptr;

	UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(NoteCanvas->AddChild(Note));
	if (!NoteSlot) return Note;

	if (!bInitializedPositions)
	{
		SetStartPoses();
		bInitializedPositions = true;
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

void URhythmSpawnWidget::UpdateNoteProgress(URhythmNoteWidget* Note, float Alpha01)
{
	if (!Note) return;
	Note->SetProgress(Alpha01);
}

void URhythmSpawnWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &ThisClass::OnViewPortResizedHandler);
	}
	LaneYPosArray.SetNum(MaxLanes);
}

void URhythmSpawnWidget::OnViewPortResizedHandler(FViewport* ViewPort, uint32)
{
	FIntPoint Size = ViewPort->GetSizeXY();
	SetStartPoses();
}

float URhythmSpawnWidget::GetLaneY(int32 LaneIndex) const
{
	if (LaneYPosArray.IsValidIndex(LaneIndex))
	{
		return LaneYPosArray[LaneIndex];
	}
	return 0;
}

void URhythmSpawnWidget::SetStartPoses()
{
	checkf(NoteCanvas, TEXT("NoteCanvas is nullptr"));

	const FGeometry CanvasGeo = NoteCanvas->GetCachedGeometry();

	// SpawnWidget의 절대 좌표계
	const FVector2D AbsPos = CanvasGeo.GetAbsolutePosition();
	const FVector2D AbsSize = CanvasGeo.GetAbsoluteSize();

	// 절대 좌표 -> 로컬 좌표
	const FVector2D TL_Local = CanvasGeo.AbsoluteToLocal(AbsPos);
	const FVector2D TR_Local = CanvasGeo.AbsoluteToLocal(AbsPos + FVector2D(AbsSize.X, 0.f));
	const FVector2D BL_Local = CanvasGeo.AbsoluteToLocal(AbsPos + FVector2D(0.f, AbsSize.Y));

	LaneXStartPos = TR_Local.X;
	LaneXEndPos = TL_Local.X;
	
	const float TopY_Local = TL_Local.Y;
	const float Height_Local = BL_Local.Y - TL_Local.Y;

	const float Step = Height_Local / static_cast<float>(MaxLanes + 1);

	for (int32 i = 0;i<MaxLanes;++i)
	{
		// 등분선: 위에서 (LaneIndex+1)칸 내려온 선을 lane 센터로 사용
		const float LaneCenterY = TopY_Local + Step * static_cast<float>(i + 1);

		if (LaneYPosArray.IsValidIndex(i))
		{
			LaneYPosArray[i] = LaneCenterY;
			UE_LOG(LogTemp, Warning, TEXT("SetStartYPos : %f"), LaneCenterY);
		}
	}
	
}
