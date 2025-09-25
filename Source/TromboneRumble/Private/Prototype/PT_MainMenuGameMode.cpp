// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProtoType/PT_MainMenuGameMode.h"
#include "ProtoType/PT_MainMenuPlayerController.h"
#include "UObject/ConstructorHelpers.h"

APT_MainMenuGameMode::APT_MainMenuGameMode()
{
	PlayerControllerClass = APT_MainMenuPlayerController::StaticClass();
}