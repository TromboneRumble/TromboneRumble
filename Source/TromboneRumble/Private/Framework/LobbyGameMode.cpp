// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/LobbyGameMode.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Framework/LobbyGameState.h"
#include "Framework/LobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

ALobbyGameMode::ALobbyGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	const FString InGameMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_InGameMap);
	checkf(!InGameMapPath.IsEmpty(), TEXT("InGameMapPath map path not found. Please set it in GameMapDeveloperSettings."));
	CachedInGameMapPath = InGameMapPath;

	LobbyGameState = GetGameState<ALobbyGameState>();
	if (!LobbyGameState)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("LobbyGameState is null"));
	}
	SetLobbyState(ELobbyState::WaitingForPlayers);
}

void ALobbyGameMode::Logout(AController* ExitedPlayer)
{
	Super::Logout(ExitedPlayer);

	const int32 CurrentPlayers = GetNumPlayers();
	
	const FString DebugPlayerName = ExitedPlayer->GetPlayerState<APlayerState>()->GetPlayerName();
	const FString DebugMsg = FString::Printf(TEXT("Player Left: %s, Total Players: %d"), *DebugPlayerName, CurrentPlayers);
	PRINT_WITH_CURRENT_CONTEXT(DebugMsg);

	if (CurrentPlayers >= MaxPlayers) return;

	const ELobbyState CurrentLobbyState = LobbyGameState->GetLobbyState();
	if (CurrentLobbyState == ELobbyState::CountdownToScramble || CurrentLobbyState == ELobbyState::InstrumentScramble)
	{
		SetLobbyState(ELobbyState::WaitingForPlayers);
	}
}

void ALobbyGameMode::OnClientIsReady(const APlayerController* ReadyPlayer)
{
	if (!ReadyPlayer) return;

	const FString PlayerName = ReadyPlayer->PlayerState->GetPlayerName();
	PRINT_WITH_CURRENT_CONTEXT(TEXT("Player is Ready: ") + PlayerName);

	if (CheckAllClientsReady())
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("All Clients Ready"));
		SetLobbyState(ELobbyState::CountdownToScramble);
	}
}

bool ALobbyGameMode::CheckAllClientsReady()
{
	const int32 CurrentPlayers = GetNumPlayers();
	if (CurrentPlayers == 0 || CurrentPlayers < MaxPlayers) return false;
	
	for (APlayerState* PS : GetGameState<AGameStateBase>()->PlayerArray)
	{
		if (PS)
		{
			const ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PS);
			if (!LobbyPlayerState || !LobbyPlayerState->IsReady()) return false;
		}
	}

	return true;
}

void ALobbyGameMode::SetLobbyState(const ELobbyState NewState)
{
	const ELobbyState CurrentLobbyState = LobbyGameState->GetLobbyState();
	if (CurrentLobbyState == NewState) return;

	PRINT_WITH_CURRENT_CONTEXT(UEnum::GetValueAsString(NewState));
	LobbyGameState->SetLobbyState(NewState);
	
	switch (NewState)
	{
		case ELobbyState::WaitingForPlayers:
			if (GetWorldTimerManager().IsTimerActive(LobbyTimerHandle))
			{
				GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
			}
			// TODO: 만약 벽이 제거되었다면 다시 생성
			break;
	            
		case ELobbyState::CountdownToScramble:
			StartTimer([this]() { SetLobbyState(ELobbyState::InstrumentScramble); });
			break;
	            
		case ELobbyState::InstrumentScramble:
			// TODO : 벽 제거 및 악기 모두 소유했는지 확인 후 다음 상태로
			// StartTimer([this]() { ServerTravelToInGame(); });
			ServerTravelToInGame();
			break;
	            
		case ELobbyState::CountdownToTravel:
			StartTimer([this]() { ServerTravelToInGame(); });
			break;
			
		default: ;
	}
}

void ALobbyGameMode::ServerTravelToInGame() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Debug::Print(TEXT("World is null from [StartGameButtonClicked]"));
		return;
	}

	// 전달받은 문자열이 "오브젝트 경로(/A/B.Map.Map)"면 패키지 경로("/A/B.Map")로 정규화
	FString PackagePath = CachedInGameMapPath;
	if (PackagePath.Contains(TEXT(".")))
	{
		PackagePath = FSoftObjectPath(CachedInGameMapPath).GetLongPackageName();
		if (PackagePath.IsEmpty())
		{
			Debug::Print(TEXT("Invalid MapPath from [StartGameButtonClicked]"));
			return;
		}
	}

	//호스트(리스닝 서버)라면 연결 중인 모든 클라와 함께 이동
	if (World->GetAuthGameMode() == nullptr)
	{
		Debug::Print(TEXT("StartGame can be called only on host from [StartGameButtonClicked]"));
		return;
	}

	if (!World->ServerTravel(PackagePath))
	{
		Debug::Print(TEXT("ServerTravel failed from [StartGameButtonClicked]"));
	}
}

void ALobbyGameMode::StartTimer(TFunction<void()> OnTimerFinished)
{
	GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
	GetWorldTimerManager().SetTimer(LobbyTimerHandle, MoveTemp(OnTimerFinished),Timer, false);
}