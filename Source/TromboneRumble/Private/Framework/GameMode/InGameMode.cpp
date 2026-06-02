// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/GameMode/InGameMode.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/InGameState.h"
#include "EasyOnlineSession.h"
#include "EasySessionTypes.h"
#include "EasySessionUtils.h"
#include "Characters/DefaultPlayerController.h"
#include "Framework/TromboneGameInstance.h"
#include "Utilities/TromboneStatics.h"

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

void AInGameMode::Logout(AController* ExitedPlayer)
{
	Super::Logout(ExitedPlayer); // 내부에서 UnregisterPlayer 호출 → NumOpenPublicConnections 즉시 갱신됨

	AInGameState* GS = GetGameState<AInGameState>();
	if (!GS || GS->GetCurrentGameState() == EInGameState::End)
	{
		return;
	}

	FEasyNamedSession CurrentGameSession;
	if (UEasyOnlineSession* EasySession = UEasyOnlineSession::Get(this))
	{
		EasySession->GetSession(NAME_GameSession, CurrentGameSession);
	}
	SessionPlayerNumber = FMath::Max(1, UEasyStatics::GetNamedSessionPlayerCount(CurrentGameSession));

	if (RhythmGameEndedPlayerCount >= SessionPlayerNumber)
	{
		if (AInGameState* GS = GetGameState<AInGameState>())
		{
			GS->Multicast_BroadCastInGameStateChanged(EInGameState::End);
		}
	}
}

void AInGameMode::OnRhythmGameEndedReport()
{
	RhythmGameEndedPlayerCount++;
	if (RhythmGameEndedPlayerCount >= SessionPlayerNumber)
	{
		if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
		{
			GI->SaveResultSceneData();
		}
		
		if (AInGameState* GS = GetGameState<AInGameState>())
		{
			GS->Multicast_BroadCastInGameStateChanged(EInGameState::End);
			
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(It->Get()))
				{
					if (!PC->IsLocalController())
					{
						PC->Client_RequestTravelToResultLevelAndLeaveSession(); 
					}
				}
			}
		}
	}
}

void AInGameMode::OnClientTravelToResultLevelAndLeaveSession()
{
	ClientsTravelToResultSceneCount++;
    
	if (ClientsTravelToResultSceneCount >= SessionPlayerNumber - 1)
	{
		FTimerHandle ServerLeaveTimer;
		GetWorldTimerManager().SetTimer(ServerLeaveTimer, FTimerDelegate::CreateLambda([this]()
		{
			if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
			{
				OnlineSession->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateLambda([this](FName Name, bool bSuccess)
				{
					UTromboneStatics::OpenLevel(this, ELevelType::ResultScene);
				}));
			}
		}), 0.5f, false);
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
		if (AInGameState* GS = GetGameState<AInGameState>())
		{
			GS->Multicast_BroadCastInGameStateChanged(EInGameState::Play);
		}
	}
}
