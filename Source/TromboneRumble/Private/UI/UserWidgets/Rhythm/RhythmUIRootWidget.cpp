// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"


void URhythmUIRootWidget::PrepareNoteContainer(const EInstrumentType& InType)
{
	if (RhythmSpawnWidget)
	{
		RhythmSpawnWidget->PrepareNoteContainer(InType);
	}
}
