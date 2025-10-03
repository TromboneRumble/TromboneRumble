// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMode.h"
AInGameMode::AInGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/InGame/BP_DefaultCharacter.BP_DefaultCharacter_C"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
