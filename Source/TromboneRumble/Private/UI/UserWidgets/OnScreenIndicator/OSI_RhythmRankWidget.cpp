// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/OnScreenIndicator/OSI_RhythmRankWidget.h"

#include "Subsystems/RhythmSubsystem.h"

void UOSI_RhythmRankWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (!IsDesignTime())
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UOSI_RhythmRankWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnRhythmGameStateChanged.RemoveDynamic(this, &ThisClass::HandleRhythmGameStateChanged);
		RhythmSubsystem->OnRhythmGameStateChanged.AddDynamic(this, &ThisClass::HandleRhythmGameStateChanged);
	}
}

void UOSI_RhythmRankWidget::HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState)
{
	if (RhythmGameState == ERhythmGameState::Ended)
	{
		RemoveFromParent();
	}
}
