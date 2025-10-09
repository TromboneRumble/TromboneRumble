// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/LobbyPlayerState.h"
#include "Framework/LobbyGameState.h"
#include "Net/UnrealNetwork.h"

class ALobbyGameState;

ALobbyPlayerState::ALobbyPlayerState()
{
	bIsReady = false;
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, bIsReady);
}

void ALobbyPlayerState::OnRep_IsReady()
{
	// TODO : UI 갱신 등의 작업
}

void ALobbyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->UpdatePlayerList();
	}
}

void ALobbyPlayerState::SetIsReady(bool bReady)
{
	if (!HasAuthority() || bIsReady == bReady) return;
	
	bIsReady = bReady;
	OnRep_IsReady();
}