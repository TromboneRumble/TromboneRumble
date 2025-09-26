// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/PT_MainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

APT_MainMenuPlayerController::APT_MainMenuPlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWidgetClassFinder(TEXT("/Game/Blueprints/Prototype/WBP_PT_Main.WBP_PT_Main_C"));
	if (MainMenuWidgetClassFinder.Succeeded())
	{
		UIMainClass = MainMenuWidgetClassFinder.Class;
	}
}

void APT_MainMenuPlayerController::BeginPlay()
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

void APT_MainMenuPlayerController::Server_RequestStartGame_Implementation()
{
	bool isSuccess = GetWorld()->ServerTravel("/Game/Levels/ThirdPersonMap?listen");
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("ServerTravel %s"), isSuccess ? TEXT("Success") : TEXT("Failed")));
}