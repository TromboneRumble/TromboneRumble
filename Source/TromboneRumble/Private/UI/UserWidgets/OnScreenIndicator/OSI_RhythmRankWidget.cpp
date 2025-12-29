// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/OnScreenIndicator/OSI_RhythmRankWidget.h"

void UOSI_RhythmRankWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (!IsDesignTime())
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}
