// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/SessionSubsystem.h"

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