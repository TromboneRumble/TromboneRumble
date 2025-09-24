// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

AMainMenuPlayerController::AMainMenuPlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWidgetClassFinder(TEXT("/Game/Blueprints/WBP_Main.WBP_Main_C"));
	if (MainMenuWidgetClassFinder.Succeeded())
	{
		UIMainClass = MainMenuWidgetClassFinder.Class;
	}
}

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("ATRPlayerController::BeginPlay")));
	
	if (UIMainClass)
	{
		UIMain = CreateWidget<UUserWidget>(GetWorld(), UIMainClass);
		if (UIMain)
		{
			UIMain->AddToViewport();
			FInputModeUIOnly InputModeData;
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputModeData);
			bShowMouseCursor = true;
		}
	}
}

void AMainMenuPlayerController::Server_RequestStartGame_Implementation()
{
	bool isSuccess = GetWorld()->ServerTravel("/Game/Levels/ThirdPersonMap?listen");
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("ServerTravel %s"), isSuccess ? TEXT("Success") : TEXT("Failed")));
}