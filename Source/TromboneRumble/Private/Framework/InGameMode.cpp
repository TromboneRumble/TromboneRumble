// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/InGameMode.h"
#include "EngineUtils.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/InGameState.h"
#include "Framework/TromboneGameInstance.h"
#include "EasyOnlineSession.h"
#include "EasySessionTypes.h"
#include "EasySessionUtils.h"

AInGameMode::AInGameMode()
{
	bUseSeamlessTravel = true;
}

void AInGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	FEasyNamedSession CurrentGameSession;
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
	OnlineSession->GetSession(NAME_GameSession, CurrentGameSession);
	if (CurrentGameSession.IsValid())
	{
		SessionPlayerNumber = UEasyStatics::GetNamedSessionPlayerCount(CurrentGameSession);
	}

	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		SessionPlayerNumber = 1;
	}
}

void AInGameMode::GameEnd() const
{
	if (AInGameState* GS = GetGameState<AInGameState>())
	{
		GS->Multicast_BroadCastInGameStateChanged(EInGameState::End);
	}
}

void AInGameMode::OnRhythmGameEndedReport()
{
	RhythmGameEndedPlayerCount++;
	if (RhythmGameEndedPlayerCount >= SessionPlayerNumber)
	{
		GameEnd();
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
	if (CurrentPlayerCount < SessionPlayerNumber)
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
