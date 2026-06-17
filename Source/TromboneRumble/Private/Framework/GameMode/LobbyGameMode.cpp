#include "Framework/GameMode/LobbyGameMode.h"
#include "AkGameplayStatics.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Framework/LobbyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/RhythmSongDataRow.h"
#include "Framework/DefaultPlayerState.h"
#include "Framework/TromboneGameInstance.h"
#include "Items/WeaponBase.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"
#include "Utilities/EnumHelper.h"

ALobbyGameMode::ALobbyGameMode()
{
	RegisteredPlayerCount = 0;
	SpawnedInstrumentCount = 0;
	EquippedInstrumentCount = 0;
	DelayTime = 5.0f;
	
	LobbyReadyPlayers.Empty();
}

void ALobbyGameMode::HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem)
{
	if (!IsValid(EquippedPlayer) || !EquippedItem) return;
	
	if (const AWeaponBase* Weapon = Cast<AWeaponBase>(EquippedItem))
	{
		if (Weapon->GetWeaponType() == EWeaponType::Headbutt)
		{
			return;
		}
	}
	
	if (++EquippedInstrumentCount >= SpawnedInstrumentCount)
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
	if (!LobbyGameState)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("LobbyGameState not found!"));
		return;
	}
	
	RegisteredPlayerCount = GetNumPlayers();
	
	
	if (UGameStateSubsystem* GS = GetGameInstance()->GetSubsystem<UGameStateSubsystem>())
	{
		GS->OnPlayerLoadingScreenFinished.AddUObject(this, &ThisClass::HandlePlayerLoadingScreenFinished);
		
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC && PC->IsLocalController())
			{
				HandlePlayerLoadingScreenFinished(PC);
			}
		}
	}
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	RegisteredPlayerCount = GetNumPlayers();
}

void ALobbyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGameStateSubsystem* GS = GameInstance->GetSubsystem<UGameStateSubsystem>())
		{
			GS->OnPlayerLoadingScreenFinished.RemoveAll(this);
		}
	}
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ALobbyGameMode::Logout(AController* ExitedPlayer)
{
	Super::Logout(ExitedPlayer);
	
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (const UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>())
			{
				GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
				
				for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
				{
					if (const APlayerController* PC = It->Get())
					{
						if (ADefaultPlayerState* PS = PC->GetPlayerState<ADefaultPlayerState>())
						{
							PS->EquippedWeaponClass = nullptr;
						}
					}
				}
				
				// AGameModeBase::GetNumPlayers()는 PlayerControllerList를 순회하는데,
				// RemoveController()는 Logout() 완료 후에 호출되므로 이탈 플레이어가 아직 포함됨 → -1 보정
				const int32 RemainingPlayers = GetNumPlayers() - 1;
				if (RemainingPlayers > 0)
				{
					if (RemainingPlayers < 2)
					{
						LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Player left in lobby. Returning to Main Menu."));
						const FString MainMenuMapName = GameStateSubsystem->GetLevelStringFromTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
						RequestServerTravel(MainMenuMapName);
					}
					else
					{
						LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Player left in lobby. Restarting lobby"));
						const FString LobbyMapName = GameStateSubsystem->GetLevelStringFromTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);
						RequestServerTravel(LobbyMapName);
					}
				}
			}
		}
	}
}

void ALobbyGameMode::HandlePlayerLoadingScreenFinished(APlayerController* PC)
{
	if (!PC)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Invalid PlayerController"));
		return;
	}
	
	if (LobbyReadyPlayers.Contains(PC))
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Player %s has already been marked as ready"), *PC->GetName()));
		return;
	}
	
	LobbyReadyPlayers.AddUnique(PC);
	if (LobbyReadyPlayers.Num() >= RegisteredPlayerCount)
	{
		if (LobbyGameState)
		{
			//TODO : SelectedSong 하드코딩 수정
			
			// InGameMap 노래들
			//const FGameplayTag SelectedSong = FMath::RandBool() ? TromboneGamePlayTags::Trombone_Rhythm_Song_EasyMapA : 
			//													TromboneGamePlayTags::Trombone_Rhythm_Song_EasyMapB;
			
			// 눈맵 노래
			const FGameplayTag SelectedSong = FMath::RandBool() ? TromboneGamePlayTags::Trombone_Rhythm_Song_MapC : 
																TromboneGamePlayTags::Trombone_Rhythm_Song_MapD;
			
			LobbyGameState->SetSelectedSongTag(SelectedSong);
			SpawnInstruments();
		}
	}
}

void ALobbyGameMode::SpawnInstruments()
{
	if (const UTromboneGameInstance* GameInstance = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		const auto* DataSubsystem = GameInstance->GetSubsystem<UGameDataSubsystem>();
		const FGameplayTag SongTag = GameInstance->GetSelectedSongTag();
		const FRhythmSongDataRow* SongRow = DataSubsystem->GetSongRow(SongTag);
		if (!SongRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("SongRow not found"));
			return;
		}
		
		const auto& InstrumentSounds = SongRow->InstrumentSounds;
		if (InstrumentSounds.Num() == 0)
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("No instrument sounds found for the selected song"));
			return;
		}
		
		TArray<AActor*> SpawnPointActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("InstrumentSpawnPoint"), SpawnPointActors);

		if (SpawnPointActors.Num() == 0)
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("No spawn points found for instruments"));
			return;
		}
		
		SpawnedInstrumentCount = FMath::Clamp(RegisteredPlayerCount - 1, 1, 3);
		
		for (int32 i = 0; i < SpawnedInstrumentCount; i++)
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
}

void ALobbyGameMode::SetLobbyState(const ELobbyState& InNewState)
{
	if (!LobbyGameState)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("LobbyGameState is null!"));
		return;
	}
	
	const ELobbyState CurrentState = LobbyGameState->GetCurrentLobbyState();
	if (CurrentState == InNewState)
	{
		const FString DebugMsg = FString::Printf(TEXT("Lobby is already in state: %s"), *EnumHelper::EnumToString(InNewState));
		LOG_WITH_CURRENT_CONTEXT(Warning, *DebugMsg);
		return;
	}
	
	LobbyGameState->SetLobbyState(InNewState);

	switch (InNewState)
	{
	case ELobbyState::WaitingForPlayers:
		break;

	case ELobbyState::CountdownToTravel:
		{
			GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
			GetWorldTimerManager().SetTimer(LobbyTimerHandle, this, &ThisClass::OnCountdownToTravel, DelayTime, false);
		}
		break;

	default:
		break;
	}
}

void ALobbyGameMode::RequestServerTravel(const FString& MapPath) const
{
	UWorld* World = GetWorld();
	if (!World || World->GetAuthGameMode() == nullptr) return;
	
	if (MapPath.IsEmpty()) 
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("MapPath is empty"));
		return;
	}
	
	if (!World->ServerTravel(MapPath))
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("ServerTravel failed"));
		return;
	}
}

void ALobbyGameMode::OnCountdownToTravel()
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (const UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>())
			{
				//TODO : InGame맵 이동 로직 UI로 수정
				//const FString InGameMapName = GameStateSubsystem->GetLevelStringFromTag(TromboneGamePlayTags::Trombone_Maps_InGame_Main);
				const FString InGameMapName = GameStateSubsystem->GetLevelStringFromTag(TromboneGamePlayTags::Trombone_Maps_InGame_Snow);
				RequestServerTravel(InGameMapName);
			}
		}
	}
}