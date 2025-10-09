// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/InGameMode.h"
#include "Characters/DefaultPlayerController.h"
#include "Utilities/DebugHelper.h"

AInGameMode::AInGameMode()
{
	// TODO : delete hard ref
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/InGame/BP_DefaultCharacter.BP_DefaultCharacter_C"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	static ConstructorHelpers::FClassFinder<ADefaultPlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/Characters/InGame/BP_DefaultPlayerController1.BP_DefaultPlayerController1_C"));
	if (PlayerControllerBPClass.Class != nullptr)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}
