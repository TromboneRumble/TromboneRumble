// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetCircle.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetCircle.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/DebugHelper.h"

void URhythmSpawnWidgetCircle::Init(ARhythmNoteSpawner* InNoteSpawner)
{
	Super::Init(InNoteSpawner);
	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasPanelSlot->SetAutoSize(true);
		CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasPanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		CanvasPanelSlot->SetPosition(FVector2D(0.f,0.f));
	}
}

URhythmNoteWidgetBase* URhythmSpawnWidgetCircle::SpawnPooledRhythmNoteWidget()
{
	URhythmNoteWidgetBase* Note = Super::SpawnPooledRhythmNoteWidget();
	if (!Note) return nullptr;

	// 새로 만들어진 애라면 부모가 없으니 Canvas에 붙여줌
	if (!Note->GetParent())
	{
		NoteCanvas->AddChild(Note);
	}

	if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(Note->Slot))
	{
		NoteSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		NoteSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		NoteSlot->SetAutoSize(true);
	}
	Note->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Note;
}

URhythmResultWidgetBase* URhythmSpawnWidgetCircle::SpawnPooledRhythmResultWidget(const FVector2D& SpawnPos,
	ENoteResult InResult)
{
	return Super::SpawnPooledRhythmResultWidget(SpawnPos, InResult);
	//TODO : Implement Circle Result Widget
}

void URhythmSpawnWidgetCircle::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!IsDesignTime())
	{
		if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::PlayFadeAnimation);
		}
	}
	
}

void URhythmSpawnWidgetCircle::NativeConstruct()
{
	Super::NativeConstruct();
}

void URhythmSpawnWidgetCircle::NativeDestruct()
{
	if (!IsDesignTime())
	{
		if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnInstrumentPicked.RemoveDynamic(this, &ThisClass::PlayFadeAnimation);
		}
	}
	Super::NativeDestruct();
}

void URhythmSpawnWidgetCircle::PlayFadeAnimation(EInstrumentType InType)
{
	if (InType == InstrumentType && FadeInAnim && !isShown)
	{
		Debug::Print(TEXT("Circle FadeIn Called"));
		PlayAnimation(FadeInAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
		isShown = true;
	}
	else if (InType != InstrumentType && FadeOutAnim && isShown)
	{
		Debug::Print(TEXT("Circle FadeOut Called"));
		PlayAnimation(FadeOutAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
		isShown = false;
	}
}
