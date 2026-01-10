// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MainMenuGameMode.h"
#include "AkGameplayStatics.h"

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (MenuBGMEvents.Num() > 0)
	{
		if (UAkAudioEvent* RandomBGM = MenuBGMEvents[FMath::RandRange(0, MenuBGMEvents.Num() - 1)])
		{
			UAkGameplayStatics::PostEvent(RandomBGM, this, 0 , FOnAkPostEventCallback());
		}
	}
}