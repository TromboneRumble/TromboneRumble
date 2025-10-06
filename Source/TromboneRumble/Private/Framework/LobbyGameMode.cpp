// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/LobbyGameMode.h"
#include "Framework/LobbyGameState.h"
#include "Framework/LobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"

ALobbyGameMode::ALobbyGameMode()
{
	GameStateClass = ALobbyGameState::StaticClass();
	PlayerStateClass = ALobbyPlayerState::StaticClass();
}