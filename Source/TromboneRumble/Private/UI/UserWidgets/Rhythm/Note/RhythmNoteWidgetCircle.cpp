// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetCircle.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"

void URhythmNoteWidgetCircle::Init(const EInstrumentType& InType)
{
	Super::Init(InType);
	if (FadeOutAnimation)
	{
		PlayAnimation(FadeOutAnimation, 0.f, 1, EUMGSequencePlayMode::Forward);
	}
	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasPanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasPanelSlot->SetPosition(FVector2D(0.f, 0.f));
		CanvasPanelSlot->SetAutoSize(true);
	}
	
}

void URhythmNoteWidgetCircle::UpdateNotePosition(const float InAlpha)
{
	Super::UpdateNotePosition(InAlpha);
	if (!SizeBox)
	{
		return;
	}

	const float Alpha = FMath::Clamp(InAlpha, 0.f, 1.f);
	const float CurrentRadius = FMath::Lerp(StartRadius, FinishRadius, Alpha);

	SizeBox->SetWidthOverride(CurrentRadius);
	SizeBox->SetHeightOverride(CurrentRadius);
}
