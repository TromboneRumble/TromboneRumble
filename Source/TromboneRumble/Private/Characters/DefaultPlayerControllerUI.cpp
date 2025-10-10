// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultPlayerController.h"
#include "Framework/LobbyGameMode.h"
#include "Framework/LobbyGameState.h"
#include "Framework/LobbyPlayerState.h"
#include "Prototype/PT_UIInGame.h"

ADefaultPlayerController::ADefaultPlayerController()
{
	//TODO : 하드 레퍼런싱에서 BP로 변경
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameWidgetClassFinder(TEXT("/Game/Blueprints/Prototype/WBP_PT_InGame.WBP_PT_InGame_C"));
	if (InGameWidgetClassFinder.Succeeded())
	{
		InGameUIClass = InGameWidgetClassFinder.Class;
	}
}

void ADefaultPlayerController::ShowInteractionUI(bool bShow) const
{
	if (!InGameUI) return;

	InGameUI->ShowInteractionHint(bShow);
}

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;
	
	InitializeUI();
	Server_NotifyClientReady();
}

EGameState ADefaultPlayerController::GetGameState() const
{
	const AGameStateBase* CurrentGameState = GetWorld()->GetGameState();
	
	if (CurrentGameState->IsA(ALobbyGameState::StaticClass()))
		return EGameState::Lobby;
	// if (CurrentGameState->IsA(AInGameState::StaticClass()))
		// return EGameState::InGame;
	return EGameState::Invalid;
}

void ADefaultPlayerController::InitializeUI()
{
	switch (GetGameState())
	{
		case EGameState::MainMenu:
			break;
		case EGameState::Lobby:
			InitializeLobbyUI();
			break;
		case EGameState::InGame:
			InitializeInGameUI();
			break;
		case EGameState::Invalid:
			break;
	}
}

void ADefaultPlayerController::InitializeLobbyUI()
{
	// TODO : Lobby UI Load
}

void ADefaultPlayerController::InitializeInGameUI()
{
	if (!InGameUIClass) return;
	
	InGameUI = CreateWidget<UPT_UIInGame>(GetWorld(), InGameUIClass);
	if (!InGameUI) return;
	
	InGameUI->AddToViewport();
	const FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
	bShowMouseCursor = false;

	// TODO : Set InGamePlayerState Ready
}

void ADefaultPlayerController::Server_NotifyClientReady_Implementation()
{
	ALobbyPlayerState* LobbyPlayerState = GetPlayerState<ALobbyPlayerState>();
	if (!LobbyPlayerState) return;

	LobbyPlayerState->SetIsReady(true);
	
	if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LobbyGameMode->OnClientReady(this);
	}
}