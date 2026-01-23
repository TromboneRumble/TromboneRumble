// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/TromboneGameInstance.h"
#include "AkComponent.h"

void UTromboneGameInstance::Init()
{
	Super::Init();
	
	PlayMenuBGM();
}

void UTromboneGameInstance::PlayMenuBGM()
{
	if (bIsMenuMusicPlaying || PlayMenuBGMEvents.Num() == 0) return;

	if (!GlobalBGMComponent)
	{
		GlobalBGMComponent = NewObject<UAkComponent>(this);
		GlobalBGMComponent->RegisterComponentWithWorld(GetWorld());
	}
	
	TArray<EMenuBGMType> Keys;
	PlayMenuBGMEvents.GetKeys(Keys);
	
	EMenuBGMType SelectedType = Keys[FMath::RandRange(0, Keys.Num() - 1)];
	if (UAkAudioEvent* PlayEvent = PlayMenuBGMEvents[SelectedType])
	{
		if (!GlobalBGMComponent->HasActiveEvents())
		{
			GlobalBGMComponent->PostAkEvent(PlayEvent);
			CurrentMenuBGMType = SelectedType;
			bIsMenuMusicPlaying = true;
		}
	}
}

void UTromboneGameInstance::StopMenuBGM()
{
	if (!GlobalBGMComponent || CurrentMenuBGMType == EMenuBGMType::None) return;

	if (TObjectPtr<UAkAudioEvent>* StopEventPtr = StopMenuBGMEvents.Find(CurrentMenuBGMType))
	{
		GlobalBGMComponent->PostAkEvent(*StopEventPtr);
		bIsMenuMusicPlaying = false;
		CurrentMenuBGMType = EMenuBGMType::None;
	}
}