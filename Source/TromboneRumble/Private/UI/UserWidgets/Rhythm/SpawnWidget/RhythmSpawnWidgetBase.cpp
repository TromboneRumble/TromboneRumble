// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetBase.h"
#include "UI/UserWidgets/Rhythm/ResultWidget/RhythmResultWidgetBase.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"

URhythmSpawnWidgetBase::URhythmSpawnWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, WidgetPool(*this)
{
}

void URhythmSpawnWidgetBase::Init(ARhythmNoteSpawner* InNoteSpawner)
{
	InstrumentType = InNoteSpawner->GetSpawnerType();
}

URhythmNoteWidgetBase* URhythmSpawnWidgetBase::SpawnPooledRhythmNoteWidget()
{
	if (!NoteWidgetClass || !NoteCanvas)
	{
		return nullptr;
	}

	URhythmNoteWidgetBase* Note = WidgetPool.GetOrCreateInstance<URhythmNoteWidgetBase>(NoteWidgetClass);
	if (Note)
	{
		Note->Init(this);
		return Note;
	}
	return nullptr;
}

URhythmResultWidgetBase* URhythmSpawnWidgetBase::SpawnPooledRhythmResultWidget(const FVector2D& SpawnPos, ENoteResult InNoteResult)
{
	if (!NoteResultWidgetClass || !NoteCanvas)
	{
		return nullptr;
	}

	URhythmResultWidgetBase* ResultWidget = WidgetPool.GetOrCreateInstance<URhythmResultWidgetBase>(NoteResultWidgetClass);
	if (ResultWidget)
	{
		ResultWidget->Init(this);
		return ResultWidget;
	}
	return nullptr;
}

void URhythmSpawnWidgetBase::ReleasePooledRhythmNoteWidget(URhythmNoteWidgetBase* Widget)
{
	if (!Widget)
	{
		return;
	}
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	WidgetPool.Release(Widget);
}

void URhythmSpawnWidgetBase::ReleasePooledRhythmResultWidget(URhythmResultWidgetBase* Widget)
{
	if (!Widget)
	{
		return;
	}
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	WidgetPool.Release(Widget);
}

void URhythmSpawnWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!IsDesignTime())
	{
		WidgetPool = FUserWidgetPool(*this);
		PrewarmWidgetPool();
		SetRenderOpacity(0.f);
	}
}

void URhythmSpawnWidgetBase::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	WidgetPool.ReleaseAllSlateResources();
}

void URhythmSpawnWidgetBase::PrewarmWidgetPool()
{
	if (!NoteCanvas) return;
	if (!NoteWidgetClass || !NoteResultWidgetClass) return;

	for (int32 i = 0; i < 10; ++i)
	{
		if (URhythmNoteWidgetBase* Note = WidgetPool.GetOrCreateInstance<URhythmNoteWidgetBase>(NoteWidgetClass))
		{
			Note->SetVisibility(ESlateVisibility::Collapsed);
			WidgetPool.Release(Note);
		}
	}

	for (int32 i = 0; i < 10; ++i)
	{
		if (URhythmResultWidgetBase* Result = WidgetPool.GetOrCreateInstance<URhythmResultWidgetBase>(NoteResultWidgetClass))
		{
			Result->SetVisibility(ESlateVisibility::Collapsed);
			WidgetPool.Release(Result);
		}
	}
}
