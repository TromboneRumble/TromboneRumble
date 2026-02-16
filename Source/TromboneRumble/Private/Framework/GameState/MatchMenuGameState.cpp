// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/GameState/MatchMenuGameState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "UI/HUD/MatchMenuHUD.h"

void AMatchMenuGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	
	UpdatePlayerList();
}

void AMatchMenuGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerList);
	DOREPLIFETIME(ThisClass, bIsTransitioningToInGame);
	DOREPLIFETIME(ThisClass, CurrentMatchType);
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

void AMatchMenuGameState::OnRep_IsTransitioningToInGame()
{
	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	const ABaseHUD* Hud = PC ? PC->GetHUD<ABaseHUD>() : nullptr;
	
	if (!Hud) return;
	
	if (bIsTransitioningToInGame)
	{
		Hud->PushLoadingOverlay();
	}
	else
	{
		Hud->PopLoadingOverlay();
	}
}

void AMatchMenuGameState::OnRep_CurrentMatchType()
{
	OnMatchTypeChanged.Broadcast(CurrentMatchType);
}

void AMatchMenuGameState::SetIsTransitioningToInGame(const bool bInIsTransitioning)
{
	if (!HasAuthority()) return;
	
	bIsTransitioningToInGame = bInIsTransitioning;
	OnRep_IsTransitioningToInGame();
}

void AMatchMenuGameState::SetMatchType(const EMatchType NewType)
{
	if (!HasAuthority()) return;

	CurrentMatchType = NewType;
	OnRep_CurrentMatchType();
}