// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetCircle.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetCircle.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/DebugHelper.h"


URhythmNoteWidgetBase* URhythmSpawnWidgetCircle::SpawnPooledRhythmNoteWidget(const EInstrumentType& InType)
{
	URhythmNoteWidgetBase* Note = Super::SpawnPooledRhythmNoteWidget(InType);
	if (!Note) return nullptr;

	if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(Note->Slot))
	{
		NoteSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		NoteSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		NoteSlot->SetAutoSize(true);
	}
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

void URhythmSpawnWidgetCircle::PlayFadeAnimation(EInstrumentType OldType, EInstrumentType NewType)
{
	if (FadeInAnim && !isShown)
	{
		PlayAnimation(FadeInAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
		isShown = true;
	}
	else if (FadeOutAnim && isShown)
	{
		PlayAnimation(FadeOutAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
		isShown = false;
	}
}
