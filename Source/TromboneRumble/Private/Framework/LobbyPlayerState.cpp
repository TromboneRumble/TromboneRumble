// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/LobbyPlayerState.h"
#include "Framework/LobbyGameState.h"

class ALobbyGameState;

void ALobbyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->UpdatePlayerList();
	}
}
