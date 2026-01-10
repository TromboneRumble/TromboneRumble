// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/LobbyGameState.h"
#include "Engine/StaticMeshActor.h"
#include "Framework/TromboneGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Utilities/Defines.h"

class UGameDataSubsystem;

void ALobbyGameState::BeginPlay()
{
	Super::BeginPlay();

	CurrentLobbyState = ELobbyState::WaitingForPlayers;
	PreviousLobbyState = ELobbyState::Invalid;
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	
	UpdatePlayerList();
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ALobbyGameState, PlayerList);
	DOREPLIFETIME(ALobbyGameState, CurrentLobbyState);
	DOREPLIFETIME(ALobbyGameState, SelectedSongTag);
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
		OnRep_PlayerList();
	}
}

void ALobbyGameState::SetLobbyState(const ELobbyState NewState)
{
	if (!HasAuthority() || CurrentLobbyState == NewState) return;

	PreviousLobbyState = CurrentLobbyState;
	CurrentLobbyState = NewState;
	OnRep_LobbyState();
}

void ALobbyGameState::SetSelectedSongTag(const FGameplayTag& InTag)
{
	if (!HasAuthority()) return;
	SelectedSongTag = InTag;
	OnRep_SelectedSongTag();
}

void ALobbyGameState::Multicast_RemoveWall_Implementation()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Wall"), FoundActors);

	if (FoundActors.Num() > 0)
	{
		if (AActor* Wall = Cast<AStaticMeshActor>(FoundActors[0]))
		{
			Wall->Destroy();
		}
	}
}

void ALobbyGameState::OnRep_PlayerList() const
{
	OnPlayerListChanged.Broadcast(PlayerList);
}

void ALobbyGameState::OnRep_LobbyState() const
{
	OnLobbyStateChanged.Broadcast(CurrentLobbyState);
}

void ALobbyGameState::OnRep_SelectedSongTag()
{
	if (UTromboneGameInstance* GameInstance = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		GameInstance->SetSelectedSongTag(SelectedSongTag);
	}

	if (UGameDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
	{
		DataSubsystem->PreloadSongAssets(SelectedSongTag);
	}
}
