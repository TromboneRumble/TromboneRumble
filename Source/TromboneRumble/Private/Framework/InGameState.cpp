// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/InGameState.h"
#include "GameFramework/PlayerState.h"
#include "Framework/DefaultPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/DebugHelper.h"

void AInGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
    OnPlayerStateAdded.Broadcast(PlayerState);
    if (ADefaultPlayerState* DefaultPS = Cast<ADefaultPlayerState>(PlayerState))
    {
        DefaultPS->OnLocalScoreChanged.AddDynamic(this, &ThisClass::HandleLocalScoreChanged);
        OnScoreChanged.Broadcast(DefaultPS);
    }
}

void AInGameState::RemovePlayerState(APlayerState* PlayerState)
{
    if (ADefaultPlayerState* DefaultPS = Cast<ADefaultPlayerState>(PlayerState))
    {
        DefaultPS->OnLocalScoreChanged.RemoveDynamic(this, &ThisClass::HandleLocalScoreChanged);
    }
    OnPlayerStateRemoved.Broadcast(PlayerState);
	Super::RemovePlayerState(PlayerState);
    OnScoreChanged.Broadcast(PlayerState);
}

void AInGameState::Multicast_BroadCastInGameStateChanged_Implementation(EInGameState InGameState)
{
    CurrentGameState = InGameState;
    OnInGameStateChanged.Broadcast(InGameState);
}

void AInGameState::HandleLocalScoreChanged(APlayerState* UpdatedPlayerState)
{
	OnScoreChanged.Broadcast(UpdatedPlayerState);
}

void AInGameState::HandleScoreChanged(APlayerState* UpdatePlayerState)
{
    RecalculateLeader();
}

void AInGameState::BeginPlay()
{
	Super::BeginPlay();
    OnScoreChanged.AddDynamic(this, &ThisClass::HandleScoreChanged);
}

void AInGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AInGameState, CurrentLeader);
    DOREPLIFETIME(AInGameState, CurrentGameState);
}

void AInGameState::RecalculateLeader()
{
    if (!HasAuthority()) return;

    APlayerState* OldLeader = CurrentLeader;
    APlayerState* NewLeader = GetTopScoringPlayer();

    if (NewLeader != OldLeader && NewLeader->GetScore()>0.f)
    {
        CurrentLeader = NewLeader;
        OnLeaderChanged.Broadcast(NewLeader, OldLeader);
    }
}

void AInGameState::OnRep_CurrentLeader(APlayerState* OldLeader)
{
    APlayerState* NewLeader = CurrentLeader;
    OnLeaderChanged.Broadcast(NewLeader, OldLeader);
}

void AInGameState::GetPlayersSortedByScore(TArray<APlayerState*>& OutPlayers) const
{
    OutPlayers = PlayerArray;

    OutPlayers.Sort([](const APlayerState& A, const APlayerState& B)
        {
            // 점수 내림차순
            if (A.GetScore() == B.GetScore())
            {
                return A.GetPlayerId() < B.GetPlayerId();
            }
            return A.GetScore() > B.GetScore();
        });
}

int32 AInGameState::GetPlayerRank(APlayerState* Player) const
{
    if (!Player)
    {
        return INDEX_NONE;
    }

    TArray<APlayerState*> Sorted;
    GetPlayersSortedByScore(Sorted);

    for (int32 Index = 0; Index < Sorted.Num(); ++Index)
    {
        if (Sorted[Index] == Player)
        {
            // 랭크는 1부터 시작
            return Index + 1;
        }
    }

    return INDEX_NONE;
}

APlayerState* AInGameState::GetTopScoringPlayer() const
{
    TArray<APlayerState*> Sorted;
    GetPlayersSortedByScore(Sorted);

    if (Sorted.Num() > 0)
    {
        return Sorted[0];
    }
    return nullptr;
}
