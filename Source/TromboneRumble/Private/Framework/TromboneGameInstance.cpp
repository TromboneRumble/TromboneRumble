// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/TromboneGameInstance.h"
#include "AkGameplayStatics.h"

void UTromboneGameInstance::OnStart()
{
	Super::OnStart();
	
	PlayMenuBGM();
}

void UTromboneGameInstance::PlayMenuBGM()
{
	if (bIsMenuMusicPlaying || PlayMenuBGMEvents.Num() == 0) return;
	
	TArray<EMenuBGMType> Keys;
	PlayMenuBGMEvents.GetKeys(Keys);
	EMenuBGMType SelectedType = Keys[FMath::RandRange(0, Keys.Num() - 1)];

	if (UAkAudioEvent* PlayEvent = PlayMenuBGMEvents[SelectedType])
	{
		UAkGameplayStatics::PostEvent(PlayEvent, nullptr, 0, FOnAkPostEventCallback());
		CurrentMenuBGMType = SelectedType;
		bIsMenuMusicPlaying = true;
	}
}

void UTromboneGameInstance::StopMenuBGM()
{
	if (CurrentMenuBGMType == EMenuBGMType::None) return;

	if (TObjectPtr<UAkAudioEvent>* StopEventPtr = StopMenuBGMEvents.Find(CurrentMenuBGMType))
	{
		UAkGameplayStatics::PostEvent(*StopEventPtr, nullptr, 0, FOnAkPostEventCallback());
        
		bIsMenuMusicPlaying = false;
		CurrentMenuBGMType = EMenuBGMType::None;
	}
}