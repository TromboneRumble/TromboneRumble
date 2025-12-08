// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetBase.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void URhythmNoteWidgetBase::Init(URhythmSpawnWidgetBase* InOwner)
{
	if (!InOwner) return;
	OwnerSpawnWidget = InOwner;
	if (!GetParent())
	{
		UCanvasPanel* ParentCanvas = InOwner->NoteCanvas;
		ParentCanvas->AddChild(this);
	}
	SetVisibility(ESlateVisibility::HitTestInvisible);
}
