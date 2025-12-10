// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetSquare.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetSquare.h"
#include "UI/UserWidgets/Rhythm/ResultWidget/RhythmResultWidgetSquare.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Subsystems/RhythmSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Utilities/Defines.h"
#include "Utilities/DebugHelper.h"

URhythmNoteWidgetBase* URhythmSpawnWidgetSquare::SpawnPooledRhythmNoteWidget(const EInstrumentType& InType)
{
	if (!bInitializedPositions)
	{
		SetStartPoses();
		bInitializedPositions = true;
	}
	URhythmNoteWidgetBase* Note = Super::SpawnPooledRhythmNoteWidget(InType);
	if (!Note) return nullptr;
	URhythmNoteWidgetSquare* SquareNote = Cast<URhythmNoteWidgetSquare>(Note);
	if (!SquareNote) return Note;
	

	// 새로 만들어진 애라면 부모가 없으니 Canvas에 붙여줌
	if (!SquareNote->GetParent())
	{
		NoteCanvas->AddChild(Note);
	}

	if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(SquareNote->Slot))
	{
		NoteSlot->SetAnchors(FAnchors(0.f, 0.f));
		NoteSlot->SetAlignment(FVector2D(0.f, 0.f));
		NoteSlot->SetAutoSize(true);
	}
	SquareNote->SetStartPoses(LaneXStartPos, LaneXEndPos, LaneYPosArray);
	SquareNote->SetVisibility(ESlateVisibility::HitTestInvisible);
	return SquareNote;
}

URhythmResultWidgetBase* URhythmSpawnWidgetSquare::SpawnPooledRhythmResultWidget(const FVector2D& SpawnPos, ENoteResult InResult)
{
	URhythmResultWidgetBase* ResultWidgetBase = Super::SpawnPooledRhythmResultWidget(SpawnPos, InResult);
	URhythmResultWidgetSquare* SquareWidget = Cast<URhythmResultWidgetSquare>(ResultWidgetBase);
	if (!SquareWidget) return ResultWidgetBase;

	if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(SquareWidget->Slot))
	{
		NoteSlot->SetAnchors(FAnchors(0.f, 0.f));
		NoteSlot->SetAlignment(FVector2D(0.f, 0.f));
		NoteSlot->SetAutoSize(true);
		NoteSlot->SetPosition(SpawnPos);
	}

	SquareWidget->PlayAnimationOnResult(InResult);
	return SquareWidget;
}

void URhythmSpawnWidgetSquare::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!IsDesignTime())
	{

		if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::PlayFadeAnimation);
		}
		if (GEngine && GEngine->GameViewport)
		{
			ViewportResizedHandle = GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &ThisClass::OnViewPortResizedHandler);
			bViewportBound = true;
		}
		LaneYPosArray.SetNum(MaxLanes);
	}
}

void URhythmSpawnWidgetSquare::NativeConstruct()
{
	Super::NativeConstruct();
}

void URhythmSpawnWidgetSquare::NativeDestruct()
{
	if (bViewportBound && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->Viewport->ViewportResizedEvent.Remove(ViewportResizedHandle);
		bViewportBound = false;
	}
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnInstrumentPicked.RemoveDynamic(this, &ThisClass::PlayFadeAnimation);
	}
	Super::NativeDestruct();
}

void URhythmSpawnWidgetSquare::PlayFadeAnimation(EInstrumentType OldType, EInstrumentType NewType)
{
	if (FadeInAnim && EInstrumentType::Background < NewType && NewType < EInstrumentType::None && !isShown)
	{
		StopAllAnimations();
		PlayAnimation(FadeInAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
	}
	else if (FadeOutAnim && NewType == EInstrumentType::Background && isShown)
	{
		StopAllAnimations();
		PlayAnimation(FadeOutAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
	}
}

void URhythmSpawnWidgetSquare::OnViewPortResizedHandler(FViewport* ViewPort, uint32)
{
	FIntPoint Size = ViewPort->GetSizeXY();
	SetStartPoses();
}

void URhythmSpawnWidgetSquare::SetStartPoses()
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

	for (int32 i = 0; i < MaxLanes; ++i)
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
