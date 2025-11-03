// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/DefaultPlayerState.h"
#include "Framework/LobbyGameState.h"
#include "Net/UnrealNetwork.h"

class ALobbyGameState;

ADefaultPlayerState::ADefaultPlayerState()
{
	bIsReady = false;
}

void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADefaultPlayerState, bIsReady);
	DOREPLIFETIME(ADefaultPlayerState, EquippedInstrumentClass);
}

void ADefaultPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->UpdatePlayerList();
	}
}

void ADefaultPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (ADefaultPlayerState* DefaultPS = Cast<ADefaultPlayerState>(PlayerState))
	{
		DefaultPS->EquippedInstrumentClass = this->EquippedInstrumentClass;
		DefaultPS->bIsReady = this->bIsReady;
	}
}

void ADefaultPlayerState::SetIsReady(bool bReady)
{
	if (!HasAuthority() || bIsReady == bReady) return;
	
	bIsReady = bReady;
}
