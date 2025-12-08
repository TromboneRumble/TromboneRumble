// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetCircle.h"

#include "Components/SizeBox.h"

void URhythmNoteWidgetCircle::Init(URhythmSpawnWidgetBase* InOwner)
{
	Super::Init(InOwner);
	if (FadeOutAnimation)
	{
		PlayAnimation(FadeOutAnimation, 0.f, 1, EUMGSequencePlayMode::Forward);
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
