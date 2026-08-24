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

	// 나간 사람의 보고는 더 이상 유효하지 않다. 남겨두면 남은 인원과 수가 어긋난다
	if (APlayerController* ExitedPC = Cast<APlayerController>(ExitedPlayer))
	{
		RhythmEndedPlayers.Remove(ExitedPC);
	}

	// 이미 종료 시퀀스 중이라면, 나간 사람을 기다리다 멈춘 이동 조건을 다시 본다
	if (bEndSequenceStarted)
	{
		TryTravelToResultLevel();
		return;
	}

	AInGameState* GS = GetGameState<AInGameState>();
	if (!GS || GS->GetCurrentGameState() == EInGameState::End)
	{
		return;
	}

	const int CurrentSessionPlayerCount = GetExpectedPlayerCount();

	// End만 알리면 입력만 막힌 채 아무도 결과 레벨로 못 간다. 정식 종료 시퀀스를 돌린다
	if (RhythmEndedPlayers.Num() >= CurrentSessionPlayerCount)
	{
		StartEndSequence(CurrentSessionPlayerCount);
	}
}

void AInGameMode::OnRhythmGameEndedReport(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	const int CurrentSessionPlayerCount = GetExpectedPlayerCount();
	
	RhythmEndedPlayers.AddUnique(PC);

	// 이미 시작했다면 중복 보고다. 다시 돌면 Client RPC가 두 번 나가 카운터가 부풀고 호스트가 먼저 떠난다
	if (bEndSequenceStarted || RhythmEndedPlayers.Num() < CurrentSessionPlayerCount)
	{
		return;
	}

	StartEndSequence(CurrentSessionPlayerCount);
}

int32 AInGameMode::GetExpectedPlayerCount() const
{
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		return 1;
	}

	// 세션이 없으면 0이 온다. 그대로 쓰면 종료 조건이 항상 참이 되고 호스트 자가 트리거는 반대로 안 걸린다.
	const int32 SessionCount = UEasyStatics::GetCurrentGameSessionPlayerCount(this);
	return SessionCount > 0 ? SessionCount : GetWorld()->GetNumPlayerControllers();
}

void AInGameMode::StartEndSequence(int32 InSessionPlayerCount)
{
	bEndSequenceStarted = true;

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

	// 혼자면 보고해줄 클라가 없으므로 호스트가 스스로 이동을 건다
	if (InSessionPlayerCount <= 1)
	{
		OnClientTravelToResultLevelAndLeaveSession();
	}
}

void AInGameMode::OnClientTravelToResultLevelAndLeaveSession()
{
	ClientsTravelToResultSceneCount++;
	TryTravelToResultLevel();
}

void AInGameMode::TryTravelToResultLevel()
{
	// 예약은 한 번뿐이다. 두 번 돌면 세션 파괴와 레벨 이동이 겹친다
	if (!bEndSequenceStarted || bResultTravelStarted)
	{
		return;
	}
	
	if (ClientsTravelToResultSceneCount < GetExpectedPlayerCount() - 1)
	{
		return;
	}

	bResultTravelStarted = true;

	// WeakLambda라야 월드가 먼저 사라졌을 때 실행되지 않고, EndPlay의 ClearAllTimersForObject에도 잡힌다
	GetWorldTimerManager().SetTimer(TimerHandle_TravelToResultLevel, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
		{
			OnlineSession->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateWeakLambda(this, [this](FName /*SessionName*/, bool /*bSuccess*/)
			{
				if (const UResultSceneSubsystem* ResultSubsystem = GetGameInstance()->GetSubsystem<UResultSceneSubsystem>())
				{
					ResultSubsystem->OpenResultLevel(this);
				}
			}));
		}
	}), 0.5f, false);
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

	//현재 접속한 플레이어
	const int32 CurrentPlayerCount = GameState ? GameState->PlayerArray.Num() : 0;
	if (CurrentPlayerCount < GetExpectedPlayerCount())
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
