// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/DefaultCharacterController.h"
#include "ProtoType/PT_UIInGame.h"
#include "Blueprint/UserWidget.h"

ADefaultCharacterController::ADefaultCharacterController()
{
	//TODO : 하드 레퍼런싱에서 datatable로 변경
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameWidgetClassFinder(TEXT("/Game/Blueprints/Prototype/WBP_PT_InGame.WBP_PT_InGame_C"));
	if (InGameWidgetClassFinder.Succeeded())
	{
		UIInGameClass = InGameWidgetClassFinder.Class;
	}
}

void ADefaultCharacterController::ShowInteractionUI(bool bShow) const
{
	if (!UIInGame) return;

	UIInGame->ShowInteractionHint(bShow);
}

void ADefaultCharacterController::BeginPlay()
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

void ADefaultCharacterController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);
}

void ADefaultCharacterController::OnUnPossess()
{
	Super::OnUnPossess();
}
