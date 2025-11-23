// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/LobbyGameMode.h"
#include "AkGameplayStatics.h"
#include "OnlineSessionSettings.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Framework/DefaultPlayerState.h"
#include "Framework/LobbyGameState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Items/InstrumentBase.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Subsystems/SessionSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/RhythmSongDataRow.h"
#include "Framework/TromboneGameInstance.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

ALobbyGameMode::ALobbyGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseSeamlessTravel = true;
	NumPublicConnections = 4;
	CurrentEquippedInstruments = 0;
	Timer = 5.0f; // TODO : delete magic number
}

void ALobbyGameMode::HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem)
{
	if (!EquippedPlayer || !EquippedItem) return;
	
	if (++CurrentEquippedInstruments >= NumPublicConnections - 1)
	{
		SetLobbyState(ELobbyState::CountdownToTravel);
	}
}

void ALobbyGameMode::HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem)
{
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	LobbyGameState = GetGameState<ALobbyGameState>();

	const UWorld* World = GetWorld();
	if (!World) return;
	
	const UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance) return;
	
	const USessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<USessionSubsystem>();
	if (!SessionSubsystem) return;

	const TSharedPtr<FOnlineSessionSettings> LastSetting = SessionSubsystem->GetLastSessionSettings();
	if (LastSetting.IsValid())
	{
		NumPublicConnections = LastSetting->NumPublicConnections;
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

	if (CurrentPlayers >= NumPublicConnections) return;

	const ELobbyState CurrentLobbyState = LobbyGameState->GetCurrentLobbyState();
	if (CurrentLobbyState == ELobbyState::CountdownToScramble || CurrentLobbyState == ELobbyState::InstrumentScramble)
	{
		SetLobbyState(ELobbyState::WaitingForPlayers);
	}
}

void ALobbyGameMode::NotifyClientReady(APlayerController* ReadyPlayer)
{
	if (!ReadyPlayer) return;

	if (CheckAllClientsReady())
	{
		//Todo : UI를 통해서 어떤 곡을 선택했는지 정하기
		if (LobbyGameState)
		{
			LobbyGameState->SetSelectedSongTag(TromboneGamePlayTags::Trombone_Rhythm_Song_MapA);
			InitializeInstruments();
		}
		SetLobbyState(ELobbyState::CountdownToScramble);
	}
}

void ALobbyGameMode::RequestServerTravel(const EGameState& InGameState)
{

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>())
		{
			switch (InGameState)
			{
			case EGameState::MainMenu:
				PRINT_WITH_CURRENT_CONTEXT(TEXT("MainMenu state is not supported for ServerTravel"));
				break;
			case EGameState::Lobby:
				RequestServerTravel(GameStateSubsystem->GetMapNameForGameState(EGameState::Lobby));
				break;
			case EGameState::InGame:
				RequestServerTravel(GameStateSubsystem->GetMapNameForGameState(EGameState::InGame));
				break;
			default:
				PRINT_WITH_CURRENT_CONTEXT(TEXT("Invalid GameState for ServerTravel"));
				break;
			}
		}
	}	
}
void ALobbyGameMode::InitializeInstruments() const
{
	UTromboneGameInstance* GameInstance = Cast<UTromboneGameInstance>(GetGameInstance());
	auto* DataSub = GetGameInstance()->GetSubsystem<UGameDataSubsystem>();

	FGameplayTag SongTag = GameInstance->GetSelectedSongTag();
	const FRhythmSongDataRow* SongRow = DataSub->GetSongRow(SongTag);

	if (!SongRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("SongRow not found"));
		return;
	}

	const auto& InstrumentSounds = SongRow->InstrumentSounds;
	if (InstrumentSounds.Num() == 0) return;

	
	TArray<AActor*> SpawnPointActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("InstrumentSpawnPoint"), SpawnPointActors);

	if (SpawnPointActors.Num() == 0) return;
	
	for (int32 i = 0; i < NumPublicConnections - 1; ++i)
	{
		const int32 SpawnPointIndex = i % SpawnPointActors.Num();
		const AActor* SpawnPoint = SpawnPointActors[SpawnPointIndex];
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		const FRotator SpawnRotation = SpawnPoint->GetActorRotation();

		const int32 InstrumentClassIndex = i % InstrumentSounds.Num();
		TSubclassOf<AInstrumentBase> ClassToSpawn = InstrumentSounds[InstrumentClassIndex].SpawnInstrument;

		GetWorld()->SpawnActor<AInstrumentBase>(ClassToSpawn, SpawnLocation, SpawnRotation);
	}
}

bool ALobbyGameMode::CheckAllClientsReady()
{
	if (GetNumPlayers() < NumPublicConnections) return false;
	
	for (APlayerState* PS : GetGameState<AGameStateBase>()->PlayerArray)
	{
		if (!PS) return false;
		
		const ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(PS);
		if (!DPS || !DPS->IsReady()) return false;
	}
	return true;
}

void ALobbyGameMode::SetLobbyState(const ELobbyState& InNewState)
{
	if (LobbyGameState->GetCurrentLobbyState() == InNewState) return;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>())
		{
			LobbyGameState->SetLobbyState(InNewState);

			switch (InNewState)
			{
			case ELobbyState::WaitingForPlayers:

				if (GetWorldTimerManager().IsTimerActive(LobbyTimerHandle))
				{
					GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
				}
				RequestServerTravel(GameStateSubsystem->GetMapNameForGameState(EGameState::Lobby));
				break;

			case ELobbyState::CountdownToScramble:
				RequestSetTimer([this]()
				{
					SetLobbyState(ELobbyState::InstrumentScramble);
				});
				break;

			case ELobbyState::InstrumentScramble:
				LobbyGameState->Multicast_RemoveWall();
				break;

			case ELobbyState::CountdownToTravel:
				RequestSetTimer([this, GameStateSubsystem]() { 
					RequestServerTravel(GameStateSubsystem->GetMapNameForGameState(EGameState::InGame)); });
				break;

			default:;
			}
		}
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