// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/InGameMode.h"
#include "ProtoType/PT_PlayerController.h"

AInGameMode::AInGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Arts/BP_Character.BP_Character_C"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PlayerControllerClass = APT_PlayerController::StaticClass();
}
