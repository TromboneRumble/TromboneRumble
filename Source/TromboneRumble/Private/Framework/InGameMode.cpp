// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/InGameMode.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/InGameState.h"
#include "Utilities/DebugHelper.h"

AInGameMode::AInGameMode()
{
	bUseSeamlessTravel = true;
}

void AInGameMode::HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem)
{
}

void AInGameMode::HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem)
{
}

void AInGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			if (FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession))
			{
				NumPublicConnections = Session->SessionSettings.NumPublicConnections;
			}
		}
	}

	if (NumPublicConnections <= 0)
	{
		NumPublicConnections = 4;
	}
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		NumPublicConnections = 1;
	}
}

void AInGameMode::HandlePlayerLoadingFinished(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	//로딩이 완료된 플레이어
	InGameReadyPlayers.AddUnique(PC);

	//현재 접속한 플레이어
	const int32 CurrentPlayerCount = GameState ? GameState->PlayerArray.Num() : 0;

	if (CurrentPlayerCount < NumPublicConnections)
	{
		return;
	}

	//현재 접속한 플레이어가 로딩까지 완료되었다면
	if (InGameReadyPlayers.Num() >= CurrentPlayerCount)
	{
		if (AInGameState* InGameState = GetGameState<AInGameState>())
		{
			InGameState->Multicast_BroadCastInGameStateChanged(EInGameState::Play);
		}
	}
}
