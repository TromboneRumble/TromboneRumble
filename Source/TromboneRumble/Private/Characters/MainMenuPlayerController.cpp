// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MainMenuPlayerController.h"

void AMainMenuPlayerController::Server_RequestStartGameByPath_Implementation(const FString& PackagePath)
{
    if (UWorld* World = GetWorld())
    {
        World->ServerTravel(PackagePath + TEXT("?listen"));
    }
}
