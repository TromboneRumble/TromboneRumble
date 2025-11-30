// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/ResultWidget/RhythmResultWidgetBase.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void URhythmResultWidgetBase::Init(URhythmSpawnWidgetBase* InOwner)
{
	if (!InOwner) return;
	if (!GetParent())
	{
		UCanvasPanel* ParentCanvas = InOwner->NoteCanvas;
		ParentCanvas->AddChild(this);
	}
	SetVisibility(ESlateVisibility::HitTestInvisible);
}
