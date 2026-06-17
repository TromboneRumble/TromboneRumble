// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenu/MatchPawnSpeakerWidget.h"
#include "Components/Image.h"
#include "Components/ActorComponents/TromboneVOIPTalker.h"

void UMatchPawnSpeakerWidget::Init(UTromboneVOIPTalker* InTalker)
{
	if (WeakTalker.IsValid())
	{
		WeakTalker->OnTalkingStateChanged.RemoveDynamic(this, &ThisClass::HandleTalkingStateChanged);
	}

	WeakTalker = InTalker;
	if (!InTalker)
	{
		ApplyColor(false);
		return;
	}

	InTalker->OnTalkingStateChanged.AddDynamic(this, &ThisClass::HandleTalkingStateChanged);
	ApplyColor(InTalker->IsSpeaking());
}

void UMatchPawnSpeakerWidget::NativeDestruct()
{
	if (WeakTalker.IsValid())
	{
		WeakTalker->OnTalkingStateChanged.RemoveDynamic(this, &ThisClass::HandleTalkingStateChanged);
	}
	WeakTalker.Reset();

	Super::NativeDestruct();
}

void UMatchPawnSpeakerWidget::HandleTalkingStateChanged(bool bIsTalking)
{
	ApplyColor(bIsTalking);
}

void UMatchPawnSpeakerWidget::ApplyColor(bool bIsTalking)
{
	if (Image_Speaker)
	{
		Image_Speaker->SetColorAndOpacity(bIsTalking ? SpeakingColor : SilentColor);
	}
}
