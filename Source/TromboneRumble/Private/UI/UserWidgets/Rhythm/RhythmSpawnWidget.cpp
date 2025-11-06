// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmSpawnWidget.h"

#include "TromboneGamePlayTags.h"
#include "UI/UserWidgets/Rhythm/RhythmNoteWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Utilities/Defines.h"


URhythmNoteWidget* URhythmSpawnWidget::SpawnRhythmNoteWidget(int32 LaneIndex)
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
	return Note;
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

	//현재 존재하는 모든 RhythmNoteWidget들에게도 변경사항 전파
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	FViewportChangedMessage Message;
	Message.LaneXStartPos = LaneXStartPos;
	Message.LaneXEndPos = LaneXEndPos;
	Message.LaneYPosArray = LaneYPosArray;

	MessageSubsystem.BroadcastMessage(TromboneGamePlayTags::Trombone_Rhythm_OnLayoutChanged, Message);
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

	const float BotY_Local = BL_Local.Y;
	const float Height_Local = BL_Local.Y - TL_Local.Y;

	const float Step = Height_Local / static_cast<float>(MaxLanes + 1);

	for (int32 i = 0;i<MaxLanes;++i)
	{
		// 등분선: 아래서부터 (LaneIndex+1)칸 올라간 선을 LaneCenterY로 사용
		const float LaneCenterY = BotY_Local - Step * static_cast<float>(i + 1);

		if (LaneYPosArray.IsValidIndex(i))
		{
			LaneYPosArray[i] = LaneCenterY;
			UE_LOG(LogTemp, Warning, TEXT("SetStartYPos : %f"), LaneCenterY);
		}
	}
	
}
