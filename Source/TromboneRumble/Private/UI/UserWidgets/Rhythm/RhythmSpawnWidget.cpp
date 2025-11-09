// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmSpawnWidget.h"
#include "UI/UserWidgets/Rhythm/RhythmNoteWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "TromboneGamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Subsystems/WidgetPoolSubsystem.h"
#include "Utilities/Defines.h"


URhythmNoteWidget* URhythmSpawnWidget::GetPooledRhythmNoteWidget(int32 LaneIndex)
{

	if (!bInitializedPositions)
	{
		SetStartPoses();
		bInitializedPositions = true;
	}

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();

	if (UWidgetPoolSubsystem* WidgetPoolSubsystem = LocalPlayer->GetSubsystem<UWidgetPoolSubsystem>())
	{
		if (URhythmNoteWidget* Note = Cast<URhythmNoteWidget>(WidgetPoolSubsystem->Acquire(NoteWidgetClass, this, NoteCanvas)))
		{
			if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(Note->Slot))
			{
				//Vector2D StartPos(LaneXStartPos, GetLaneY(LaneIndex));
				NoteSlot->SetAnchors(FAnchors(0.f, 0.f));
				NoteSlot->SetAlignment(FVector2D(0.f, 0.f));
				NoteSlot->SetAutoSize(true);
				//NoteSlot->SetPosition(StartPos);
			}
			return Note;
		}
	}
	return nullptr;
}

void URhythmSpawnWidget::ReleasePooledRhythmNoteWidget(URhythmNoteWidget* Widget)
{
	if (!Widget)
	{
		return;
	}
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (UWidgetPoolSubsystem* WidgetPoolSubsystem = LocalPlayer->GetSubsystem<UWidgetPoolSubsystem>())
	{
		WidgetPoolSubsystem->Release(Widget);
	}
}

void URhythmSpawnWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!IsDesignTime())
	{
		if (UWidgetPoolSubsystem* WidgetPoolSubsystem = GetOwningLocalPlayer()->GetSubsystem<UWidgetPoolSubsystem>())
		{
			WidgetPoolSubsystem->Prewarm(NoteWidgetClass, 100, this, NoteCanvas);
		}
	}
}

void URhythmSpawnWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (GEngine && GEngine->GameViewport)
	{
		ViewportResizedHandle = GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &ThisClass::OnViewPortResizedHandler);
		bViewportBound = true;
	}
	LaneYPosArray.SetNum(MaxLanes);
}

void URhythmSpawnWidget::NativeDestruct()
{
	if (bViewportBound && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->Viewport->ViewportResizedEvent.Remove(ViewportResizedHandle);
		bViewportBound = false;
	}
	Super::NativeDestruct();
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
	
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;

	const FGeometry CanvasGeo = NoteCanvas->GetCachedGeometry();

	// SpawnWidget의 절대 좌표계
	const FVector2D TL_Abs = CanvasGeo.GetAbsolutePosition();
	const FVector2D AbsSize = CanvasGeo.GetAbsoluteSize();
	const FVector2D TR_Abs = CanvasGeo.GetAbsolutePosition() + FVector2D(AbsSize.X - 42.f, 0.f);
	

	// 절대 좌표 -> 로컬 좌표
	const FVector2D TL_Local = CanvasGeo.AbsoluteToLocal(TL_Abs);
	const FVector2D TR_Local = CanvasGeo.AbsoluteToLocal(TR_Abs);
	const FVector2D BL_Local = CanvasGeo.AbsoluteToLocal(TL_Abs + FVector2D(0.f, AbsSize.Y));

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
			LaneYPosArray[i] = LaneCenterY - 20.f;
		}
	}


	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>())
		{
			FViewportChangedMessage Message;
			Message.LaneXStartPos = LaneXStartPos;
			Message.LaneXEndPos = LaneXEndPos;
			Message.LaneYPosArray = LaneYPosArray;

			MessageSubsystem->BroadcastMessage(TromboneGamePlayTags::Trombone_Rhythm_OnLayoutChanged, Message);
		}
	}
	
}
