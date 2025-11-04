// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
#include "UI/UserWidgets/Rhythm/RhythmSpawnWidget.h"


void URhythmUIRootWidget::SpawnNotes(URhythmSpawnWidget* SpawnWidget, int32 LaneIndex)
{
	if (IsValid(SpawnWidget))
	{
		SpawnWidget->SpawnNote(LaneIndex);
	}
}
