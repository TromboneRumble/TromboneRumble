// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Utilities/DebugHelper.h"

ALobbyGameMode::ALobbyGameMode()
{
	GameStateClass = ALobbyGameState::StaticClass();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	UpdatePlayerList();
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	UpdatePlayerList();
}

void ALobbyGameMode::UpdatePlayerList() const
{
	ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>();
	if (!LobbyGameState) return;

	TArray<FString> NewPlayerList;
	for (const APlayerState* PlayerState : LobbyGameState->PlayerArray)
	{
		if (PlayerState)
		{
			NewPlayerList.Add(PlayerState->GetPlayerName());
			Debug::PrintWithCurrentContext(FString::Printf(TEXT("Player: %s"), *PlayerState->GetPlayerName()));
		}
	}

	LobbyGameState->PlayerList = NewPlayerList;
	LobbyGameState->OnRep_SessionPlayerList();
}