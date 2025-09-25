// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/InGameGameMode.h"
#include "ProtoType/InGamePlayerController.h"

AInGameGameMode::AInGameGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Arts/BP_Character.BP_Character_C"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PlayerControllerClass = AInGamePlayerController::StaticClass();
}
