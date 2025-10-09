// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/TrombonePlayerController.h"
#include "Framework/InGameState.h"
#include "Framework/LobbyGameMode.h"
#include "Framework/LobbyGameState.h"
#include "Framework/LobbyPlayerState.h"
#include "Prototype/PT_UIInGame.h"
#include "Utilities/DebugHelper.h"

ATrombonePlayerController::ATrombonePlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameWidgetClassFinder(TEXT("/Game/Blueprints/Prototype/WBP_PT_InGame.WBP_PT_InGame_C"));
	if (InGameWidgetClassFinder.Succeeded())
	{
		InGameUIClass = InGameWidgetClassFinder.Class;
	}
}

void ATrombonePlayerController::ShowInteractionUI(bool bShow) const
{
	if (!InGameUI) return;

	InGameUI->ShowInteractionHint(bShow);
}

void ATrombonePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;
	
	InitializeUI();
	Server_NotifyClientReady();
}

EGameState ATrombonePlayerController::GetGameState() const
{
	const AGameStateBase* CurrentGameState = GetWorld()->GetGameState();
	
	if (CurrentGameState->IsA(ALobbyGameState::StaticClass()))
		return EGameState::Lobby;
	if (CurrentGameState->IsA(AInGameState::StaticClass()))
		return EGameState::InGame;
	return EGameState::Invalid;
}

void ATrombonePlayerController::InitializeUI()
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

void ATrombonePlayerController::InitializeLobbyUI()
{
	// TODO : Lobby UI Load
}

void ATrombonePlayerController::InitializeInGameUI()
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

void ATrombonePlayerController::Server_NotifyClientReady_Implementation()
{
	ALobbyPlayerState* LobbyPlayerState = GetPlayerState<ALobbyPlayerState>();
	if (!LobbyPlayerState) return;

	LobbyPlayerState->SetIsReady(true);
	
	if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LobbyGameMode->OnClientIsReady(this);
	}
}