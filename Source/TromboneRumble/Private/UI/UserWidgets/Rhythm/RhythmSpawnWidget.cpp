// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmSpawnWidget.h"
#include "UI/UserWidgets/Rhythm/RhythmNoteWidget.h"
#include "UI/UserWidgets/Rhythm/RhythmResultWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "TromboneGamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Utilities/Defines.h"


URhythmSpawnWidget::URhythmSpawnWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, WidgetPool(*this)
{
}

URhythmNoteWidget* URhythmSpawnWidget::GetPooledRhythmNoteWidget(int32 LaneIndex)
{

	if (!bInitializedPositions)
	{
		SetStartPoses();
		bInitializedPositions = true;
	}
	if (!NoteWidgetClass || !NoteCanvas)
	{
		return nullptr;
	}

	URhythmNoteWidget* Note = WidgetPool.GetOrCreateInstance<URhythmNoteWidget>(NoteWidgetClass);
	if (!Note)
	{
		return nullptr;
	}

	// 새로 만들어진 애라면 부모가 없으니 Canvas에 붙여줌
	if (!Note->GetParent())
	{
		NoteCanvas->AddChild(Note);
	}

	if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(Note->Slot))
	{
		NoteSlot->SetAnchors(FAnchors(0.f, 0.f));
		NoteSlot->SetAlignment(FVector2D(0.f, 0.f));
		NoteSlot->SetAutoSize(true);
	}

	Note->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Note;
}

void URhythmSpawnWidget::ReleasePooledRhythmNoteWidget(URhythmNoteWidget* Widget)
{
	if (!Widget)
	{
		return;
	}
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	WidgetPool.Release(Widget);
}

void URhythmSpawnWidget::ReleasePooledRhythmResultWidget(URhythmResultWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	Widget->SetVisibility(ESlateVisibility::Collapsed);
	WidgetPool.Release(Widget);
}


void URhythmSpawnWidget::SpawnRhythmResultWidget(const FVector2D& SpawnPos, ENoteResult InResult)
{
	if (!NoteResultWidgetClass || !NoteCanvas)
	{
		return;
	}

	if (URhythmResultWidget* ResultWidget = WidgetPool.GetOrCreateInstance<URhythmResultWidget>(NoteResultWidgetClass))
	{
		if (!ResultWidget->GetParent())
		{
			NoteCanvas->AddChild(ResultWidget);
		}
		ResultWidget->SetOwnerSpawnWidget(this);

		if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(ResultWidget->Slot))
		{
			NoteSlot->SetAnchors(FAnchors(0.f, 0.f));
			NoteSlot->SetAlignment(FVector2D(0.f, 0.f));
			NoteSlot->SetAutoSize(true);
			NoteSlot->SetPosition(SpawnPos);
		}

		ResultWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		ResultWidget->PlayAnimationOnResult(InResult);
	}
}


void URhythmSpawnWidget::PlayFadeAnimation(EInstrumentType InType)
{
	if (InType == InstrumentType && FadeInAnim && !isShown)
	{
		PlayAnimation(FadeInAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
		isShown = true;
	}
	else if (InType != InstrumentType && FadeOutAnim && isShown)
	{
		PlayAnimation(FadeOutAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
		isShown = false;
	}
}



void URhythmSpawnWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!IsDesignTime())
	{
		WidgetPool = FUserWidgetPool(*this);
		PrewarmWidgetPool();
		SetColorAndOpacity(FLinearColor{ 1.f,1.f,1.f,0.f });
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

void URhythmSpawnWidget::PrewarmWidgetPool()
{
	if (!NoteCanvas) return;
	if (!NoteWidgetClass || !NoteResultWidgetClass) return;

	for (int32 i = 0; i < 10; ++i)
	{
		if (URhythmNoteWidget* Note = WidgetPool.GetOrCreateInstance<URhythmNoteWidget>(NoteWidgetClass))
		{
			if (!Note->GetParent())
			{
				NoteCanvas->AddChild(Note);
			}
			Note->SetVisibility(ESlateVisibility::Collapsed);
			WidgetPool.Release(Note);
		}
	}

	for (int32 i = 0; i < 10; ++i)
	{
		if (URhythmResultWidget* Result = WidgetPool.GetOrCreateInstance<URhythmResultWidget>(NoteResultWidgetClass))
		{
			if (!Result->GetParent())
			{
				NoteCanvas->AddChild(Result);
			}
			Result->SetOwnerSpawnWidget(this);
			Result->SetVisibility(ESlateVisibility::Collapsed);
			WidgetPool.Release(Result);
		}
	}
}

void URhythmSpawnWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	WidgetPool.ReleaseAllSlateResources();
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
	const FVector2D TR_Abs = CanvasGeo.GetAbsolutePosition() + FVector2D(AbsSize.X - 41.f, 0.f);
	

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
