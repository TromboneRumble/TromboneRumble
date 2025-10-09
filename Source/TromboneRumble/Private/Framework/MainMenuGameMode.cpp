// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MainMenuGameMode.h"
#include "Characters/DefaultPlayerController.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	PlayerControllerClass = ADefaultPlayerController::StaticClass();
}
