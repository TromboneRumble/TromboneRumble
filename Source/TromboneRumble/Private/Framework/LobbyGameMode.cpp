// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/LobbyGameMode.h"
#include "AkGameplayStatics.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Framework/DefaultPlayerState.h"
#include "Framework/LobbyGameState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/RhythmSongDataRow.h"
#include "Framework/TromboneGameInstance.h"
#include "Items/WeaponBase.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

ALobbyGameMode::ALobbyGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem)
{
	if (!EquippedPlayer || !EquippedItem) return;
	
	if (const AWeaponBase* Weapon = Cast<AWeaponBase>(EquippedItem))
	{
		if (Weapon->GetWeaponType() == EWeaponType::Headbutt)
		{
			return;
		}
	}
	
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

	LobbyReadyPlayers.Empty();
	if (UGameStateSubsystem* GS = GetGameInstance()->GetSubsystem<UGameStateSubsystem>())
	{
		GS->OnPlayerLoadingScreenFinished.AddUObject(this, &ThisClass::HandlePlayerLoadingScreenFinished);
	}

	SetLobbyState(ELobbyState::WaitingForPlayers);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ADefaultPlayerState* PS = NewPlayer->GetPlayerState<ADefaultPlayerState>())
	{
		if (PS->GetSkinColor() == FLinearColor::Black)
		{
			const FLinearColor AssignedColor = AssignUniqueColorToCharacter();
			PS->SetSkinColor(AssignedColor); 
		}
	}
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

void ALobbyGameMode::RequestServerTravel(const ELevelState& InLevelState)
{
	if (GetWorldTimerManager().IsTimerActive(LobbyTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
	}
	
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>())
		{
			switch (InLevelState)
			{
			case ELevelState::MainMenu:
				PRINT_WITH_CURRENT_CONTEXT(TEXT("MainMenu state is not supported for ServerTravel"));
				break;
			case ELevelState::Lobby:
				RequestServerTravel(GameStateSubsystem->GetMapNameForTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main));
				break;
			case ELevelState::InGame:
				RequestServerTravel(GameStateSubsystem->GetMapNameForTag(TromboneGamePlayTags::Trombone_Maps_InGame_Main));
				break;
			default:
				PRINT_WITH_CURRENT_CONTEXT(TEXT("Invalid GameState for ServerTravel"));
				break;
			}
		}
	}	
}
void ALobbyGameMode::HandlePlayerLoadingScreenFinished(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	//로딩이 완료된 플레이어
	LobbyReadyPlayers.AddUnique(PC);

	//현재 접속한 플레이어
	const int32 CurrentPlayerCount = GameState ? GameState->PlayerArray.Num() : 0;

	if (CurrentPlayerCount < NumPublicConnections)
	{
		return;
	}

	//현재 접속한 플레이어가 로딩까지 완료되었다면
	if (LobbyReadyPlayers.Num() >= CurrentPlayerCount)
	{
		if (LobbyGameState)
		{
			LobbyGameState->SetSelectedSongTag(TromboneGamePlayTags::Trombone_Rhythm_Song_MapA);
			InitializeInstruments();
		}
		SetLobbyState(ELobbyState::CountdownToScramble);
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
		TSubclassOf<AWeaponBase> ClassToSpawn = InstrumentSounds[InstrumentClassIndex].SpawnInstrument;

		GetWorld()->SpawnActor<AWeaponBase>(ClassToSpawn, SpawnLocation, SpawnRotation);
	}
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
				RequestServerTravel(GameStateSubsystem->GetMapNameForTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main));
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
					RequestServerTravel(GameStateSubsystem->GetMapNameForTag(TromboneGamePlayTags::Trombone_Maps_InGame_Main));
				});
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

FLinearColor ALobbyGameMode::AssignUniqueColorToCharacter()
{
	if (!HasAuthority()) return FLinearColor::White;

	if (UsedColors.Num() < 4) 
	{
		TArray<FLinearColor> RemainingColors = AvailableColors;
        
		for (const FLinearColor& Color : UsedColors)
		{
			RemainingColors.Remove(Color);
		}

		if (RemainingColors.Num() > 0)
		{
			const int32 RandomIndex = FMath::RandRange(0, RemainingColors.Num() - 1);
			const FLinearColor AssignedColor = RemainingColors[RandomIndex];
            
			UsedColors.Add(AssignedColor); 
            
			return AssignedColor;
		}
	}

	const int32 RandomIndex = FMath::RandRange(0, AvailableColors.Num() - 1);
	return AvailableColors[RandomIndex];
}