// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/GameState/MatchMenuGameState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

void AMatchMenuGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	
	UpdatePlayerList();
}

void AMatchMenuGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerList);
}

void AMatchMenuGameState::UpdatePlayerList()
{
	if (!HasAuthority()) return;

	TArray<FString> NewPlayerList;
	for (const APlayerState* PlayerState : PlayerArray)
	{
		if (PlayerState)
		{
			NewPlayerList.Add(PlayerState->GetPlayerName());
		}
	}

	PlayerList = NewPlayerList;
    
	if (GetNetMode() != NM_Client)
	{
		OnRep_PlayerList();
	}
}

void AMatchMenuGameState::OnRep_PlayerList() const
{
	OnPlayerListChanged.Broadcast(PlayerList);
}