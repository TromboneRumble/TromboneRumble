// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/InGameState.h"

#include "Framework/DefaultPlayerState.h"


void AInGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
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
	Super::RemovePlayerState(PlayerState);
    OnScoreChanged.Broadcast(PlayerState);
}
void AInGameState::HandleLocalScoreChanged(APlayerState* UpdatedPlayerState)
{
	OnScoreChanged.Broadcast(UpdatedPlayerState);
}
