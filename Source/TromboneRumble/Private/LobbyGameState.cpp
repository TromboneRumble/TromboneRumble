// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/SessionSubsystem.h"

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	
	UpdatePlayerList();
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ALobbyGameState, PlayerList);
}

void ALobbyGameState::OnRep_SessionPlayerList() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const USessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<USessionSubsystem>())
		{
			SessionSubsystem->OnPlayerListUpdated.Broadcast(PlayerList);
		}
	}
}

void ALobbyGameState::UpdatePlayerList()
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
		OnRep_SessionPlayerList();
	}
}