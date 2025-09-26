// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/PT_PlayerController.h"
#include "ProtoType/PT_UIInGame.h"
#include "Blueprint/UserWidget.h"

APT_PlayerController::APT_PlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameWidgetClassFinder(TEXT("/Game/Blueprints/Prototype/WBP_PT_InGame.WBP_PT_InGame_C"));
	if (InGameWidgetClassFinder.Succeeded())
	{
		UIInGameClass = InGameWidgetClassFinder.Class;
	}
}

void APT_PlayerController::ShowInteractionUI(const bool bShow) const
{
	if (!UIInGame) return;

	UIInGame->ShowInteractionHint(bShow);
}

void APT_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UIInGameClass)
	{
		UIInGame = CreateWidget<UPT_UIInGame>(GetWorld(), UIInGameClass);
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
