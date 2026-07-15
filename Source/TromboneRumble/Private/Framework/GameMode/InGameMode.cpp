// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/GameMode/InGameMode.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/InGameState.h"
#include "EasyOnlineSession.h"
#include "EasySessionTypes.h"
#include "EasySessionStatics.h"
#include "Characters/DefaultPlayerController.h"
#include "Subsystems/ResultSceneSubsystem.h"

void AInGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AInGameMode::Logout(AController* ExitedPlayer)
{
	Super::Logout(ExitedPlayer); // 내부에서 UnregisterPlayer 호출 → NumOpenPublicConnections 즉시 갱신됨

	AInGameState* GS = GetGameState<AInGameState>();
	if (!GS || GS->GetCurrentGameState() == EInGameState::End)
	{
		return;
	}
	
	const int CurrentSessionPlayerCount = GetWorld()->GetNetMode() == NM_Standalone ? 1 : UEasyStatics::GetCurrentGameSessionPlayerCount(this);

	if (RhythmEndedPlayers.Num() >= CurrentSessionPlayerCount)
	{
		GS->Multicast_BroadCastInGameStateChanged(EInGameState::End);
	}
}

void AInGameMode::OnRhythmGameEndedReport(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	const int CurrentSessionPlayerCount = GetWorld()->GetNetMode() == NM_Standalone ? 1 : UEasyStatics::GetCurrentGameSessionPlayerCount(this);
	
	RhythmEndedPlayers.AddUnique(PC);
	if (RhythmEndedPlayers.Num() >= CurrentSessionPlayerCount)
	{
		if (UResultSceneSubsystem* ResultSubsystem = GetGameInstance()->GetSubsystem<UResultSceneSubsystem>())
		{
			ResultSubsystem->SaveResultSceneData();
		}
		
		if (AInGameState* GS = GetGameState<AInGameState>())
		{
			GS->Multicast_BroadCastInGameStateChanged(EInGameState::End);
			
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				if (ADefaultPlayerController* EachPC = Cast<ADefaultPlayerController>(It->Get()))
				{
					if (!EachPC->IsLocalController())
					{
						EachPC->Client_RequestTravelToResultLevelAndLeaveSession();
					}
				}
			}
		}
	}
	
	if (CurrentSessionPlayerCount == 1)
	{
		OnClientTravelToResultLevelAndLeaveSession();
	}
}

void AInGameMode::OnClientTravelToResultLevelAndLeaveSession()
{
	const int CurrentSessionPlayerCount = GetWorld()->GetNetMode() == NM_Standalone ? 1 : UEasyStatics::GetCurrentGameSessionPlayerCount(this);

	ClientsTravelToResultSceneCount++;
	if (ClientsTravelToResultSceneCount >= CurrentSessionPlayerCount - 1)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_TravelToResultLevel, FTimerDelegate::CreateLambda([this]()
		{
			if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
			{
				OnlineSession->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateLambda([this](FName /*SessionName*/, bool /*bSuccess*/)
				{
					if (const UResultSceneSubsystem* ResultSubsystem = GetGameInstance()->GetSubsystem<UResultSceneSubsystem>())
					{
						ResultSubsystem->OpenResultLevel(this);
					}
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

	TryStartInGamePlay();

	if (!bInGamePlayStarted)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_RetryStartInGame,
			this, &AInGameMode::TryStartInGamePlay, 0.5f, true);
	}
}

void AInGameMode::TryStartInGamePlay()
{
	if (bInGamePlayStarted)
	{
		return;
	}

	const int CurrentSessionPlayerCount = GetWorld()->GetNetMode() == NM_Standalone ? 1 : UEasyStatics::GetCurrentGameSessionPlayerCount(this);

	//현재 접속한 플레이어
	const int32 CurrentPlayerCount = GameState ? GameState->PlayerArray.Num() : 0;
	if (CurrentPlayerCount < CurrentSessionPlayerCount)
	{
		return;
	}

	//현재 접속한 플레이어가 로딩까지 완료되었다면
	if (InGameReadyPlayers.Num() >= CurrentPlayerCount)
	{
		if (AInGameState* GS = GetGameState<AInGameState>())
		{
			bInGamePlayStarted = true;
			GetWorldTimerManager().ClearTimer(TimerHandle_RetryStartInGame);
			GS->Multicast_BroadCastInGameStateChanged(EInGameState::Play);
		}
	}
}
