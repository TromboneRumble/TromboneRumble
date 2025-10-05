// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyPlayerState.h"
#include "LobbyGameState.h"

void ALobbyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->UpdatePlayerList();
	}
}
