// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/PT_InGameMode.h"
#include "ProtoType/PT_PlayerController.h"

APT_InGameMode::APT_InGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Arts/BP_Character.BP_Character_C"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PlayerControllerClass = APT_PlayerController::StaticClass();
}
