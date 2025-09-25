// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProtoType/MainMenuGameMode.h"
#include "ProtoType/MainMenuPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();
}