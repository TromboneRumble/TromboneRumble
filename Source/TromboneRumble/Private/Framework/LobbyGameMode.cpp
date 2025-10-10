// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/LobbyGameMode.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Framework/LobbyGameState.h"
#include "Framework/LobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

ALobbyGameMode::ALobbyGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentEquippedInstruments = 0;
	MaxPlayers = 3; // TODO : delete magic number
	Timer = 5.0f; // TODO : delete magic number
	CachedInGameMapPath = TEXT("");
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	OnClientReadyDelegate.AddDynamic(this, &ALobbyGameMode::HandleClientReady);
	OnInstrumentEquippedDelegate.AddDynamic(this, &ALobbyGameMode::HandleInstrumentEquipped);
	
	LobbyGameState = GetGameState<ALobbyGameState>();
	if (!LobbyGameState)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("LobbyGameState is null"));
		return;
	}
	
	InitializeMapPath();
	InitializeInstruments();
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

	const ELobbyState CurrentLobbyState = LobbyGameState->GetCurrentLobbyState();
	if (CurrentLobbyState == ELobbyState::CountdownToScramble || CurrentLobbyState == ELobbyState::InstrumentScramble)
	{
		SetLobbyState(ELobbyState::WaitingForPlayers);
	}
}

void ALobbyGameMode::RequestServerTravel(const EGameState InGameState)
{
	switch (InGameState)
	{
		case EGameState::MainMenu:
			PRINT_WITH_CURRENT_CONTEXT(TEXT("MainMenu state is not supported for ServerTravel"));
			break;
		case EGameState::InGame:
			RequestServerTravel(CachedInGameMapPath);
			break;
		case EGameState::Lobby:
			RequestServerTravel(CachedLobbyMapPath);
			break;
		default:
			PRINT_WITH_CURRENT_CONTEXT(TEXT("Invalid GameState for ServerTravel"));
			break;
	}
}

void ALobbyGameMode::InitializeMapPath()
{
	FString InGameMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_InGameMap);
	FString LobbyMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_LobbyMap);
	
	if (InGameMapPath.IsEmpty() || LobbyMapPath.IsEmpty())
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("MapPath is empty. Please set it in GameMaps Location in Project Settings."));
	}
	
	if (InGameMapPath.Contains(TEXT("."))) // 전달받은 문자열이 "오브젝트 경로(/A/B.Map.Map)"면 패키지 경로("/A/B.Map")로 정규화
	{
		InGameMapPath = FSoftObjectPath(CachedInGameMapPath).GetLongPackageName();
	}
	if (LobbyMapPath.Contains(TEXT(".")))
	{
		LobbyMapPath = FSoftObjectPath(CachedLobbyMapPath).GetLongPackageName();
	}
	
	CachedInGameMapPath = InGameMapPath;
	CachedLobbyMapPath = LobbyMapPath;
}

void ALobbyGameMode::InitializeInstruments() const
{
	if (!InstrumentToSpawn) return;
	
	TArray<AActor*> SpawnPointActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("InstrumentSpawnPoint"), SpawnPointActors);

	if (SpawnPointActors.Num() > 0)
	{
		const AActor* SpawnPoint = SpawnPointActors[0];
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		const FRotator SpawnRotation = SpawnPoint->GetActorRotation();
		
		for (int32 i = 0; i < MaxPlayers - 1; ++i)
		{
			GetWorld()->SpawnActor<AActor>(InstrumentToSpawn, SpawnLocation, SpawnRotation);
		}
	}
}

bool ALobbyGameMode::CheckAllClientsReady()
{
	if (GetNumPlayers() < MaxPlayers) return false;
	
	for (APlayerState* PlayerState : GetGameState<AGameStateBase>()->PlayerArray)
	{
		if (!PlayerState) return false;
		
		const ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerState);
		if (!LobbyPlayerState || !LobbyPlayerState->IsReady()) return false;
	}

	return true;
}

void ALobbyGameMode::SetLobbyState(const ELobbyState NewState)
{
	if (LobbyGameState->GetCurrentLobbyState() == NewState) return;

	LobbyGameState->SetLobbyState(NewState);
	
	switch (NewState)
	{
		case ELobbyState::WaitingForPlayers:
			
			if (GetWorldTimerManager().IsTimerActive(LobbyTimerHandle))
			{
				GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
			}
			RequestServerTravel(CachedLobbyMapPath);
			break;
	            
		case ELobbyState::CountdownToScramble:
			RequestSetTimer([this]() { SetLobbyState(ELobbyState::InstrumentScramble); });
			break;
	            
		case ELobbyState::InstrumentScramble:
			LobbyGameState->Multicast_RemoveWall();
			break;
	            
		case ELobbyState::CountdownToTravel:
			RequestSetTimer([this]() { RequestServerTravel(CachedInGameMapPath); });
			break;
			
		default: ;
	}
}

void ALobbyGameMode::RequestServerTravel(const FString& MapPath) const
{
	UWorld* World = GetWorld();
	if (!World || World->GetAuthGameMode() == nullptr || MapPath.IsEmpty()) return;
	
	if (!World->ServerTravel(MapPath))
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("ServerTravel failed"));
	}
}

void ALobbyGameMode::RequestSetTimer(TFunction<void()> OnTimerFinished)
{
	GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
	GetWorldTimerManager().SetTimer(LobbyTimerHandle, MoveTemp(OnTimerFinished),Timer, false);
}

void ALobbyGameMode::HandleClientReady(APlayerController* ReadyPlayer)
{
	if (!ReadyPlayer) return;

	const FString PlayerName = ReadyPlayer->PlayerState->GetPlayerName();
	const FString DebugMsg = FString::Printf(TEXT("Player Ready: %s"), *PlayerName);
	PRINT_WITH_CURRENT_CONTEXT(DebugMsg);

	if (CheckAllClientsReady())
	{
		SetLobbyState(ELobbyState::CountdownToScramble);
	}
}

void ALobbyGameMode::HandleInstrumentEquipped(APlayerController* EquippedPlayer)
{
	if (!EquippedPlayer) return;

	const FString DebugMsg = FString::Printf(TEXT("Instrument Equipped by %s"), *EquippedPlayer->PlayerState->GetPlayerName());
	PRINT_WITH_CURRENT_CONTEXT(DebugMsg);

	if (++CurrentEquippedInstruments >= MaxPlayers - 1)
	{
		SetLobbyState(ELobbyState::CountdownToTravel);
	}
}
