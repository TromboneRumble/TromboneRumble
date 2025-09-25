// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/InGamePlayerController.h"
#include "ProtoType/UIInGame.h"
#include "Blueprint/UserWidget.h"

AInGamePlayerController::AInGamePlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameWidgetClassFinder(TEXT("/Game/Blueprints/WBP_InGame.WBP_InGame_C"));
	if (InGameWidgetClassFinder.Succeeded())
	{
		UIInGameClass = InGameWidgetClassFinder.Class;
	}
}

void AInGamePlayerController::ShowInteractionUI(const bool bShow) const
{
	if (!UIInGame) return;

	UIInGame->ShowInteractionHint(bShow);
}

void AInGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UIInGameClass)
	{
		UIInGame = CreateWidget<UUIInGame>(GetWorld(), UIInGameClass);
		if (UIInGame)
		{
			UIInGame->AddToViewport();
			const FInputModeGameOnly InputModeData;
			SetInputMode(InputModeData);
			bShowMouseCursor = false;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create UIInGame"));
		}
	}
}
