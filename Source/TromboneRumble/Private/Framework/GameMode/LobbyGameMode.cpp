// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Framework/GameMode/LobbyGameMode.h"
#include "EasySessionStatics.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Components/ActorComponents/LobbyDirectorComponent.h"
#include "Components/ActorComponents/PlayerReadyCheckComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/DebugHelper.h"

ALobbyGameMode::ALobbyGameMode()
{
	LobbyDirector = CreateDefaultSubobject<ULobbyDirectorComponent>(TEXT("LobbyDirector"));
	PlayerReadyCheck = CreateDefaultSubobject<UPlayerReadyCheckComponent>(TEXT("PlayerReadyCheck"));
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	PlayerReadyCheck->OnPlayerReady.AddUObject(LobbyDirector, &ULobbyDirectorComponent::HandlePlayerReady);
	PlayerReadyCheck->OnAllPlayersReady.AddUObject(LobbyDirector, &ULobbyDirectorComponent::HandleAllPlayersReady);
	LobbyDirector->OnTravelCountdownFinished.AddUObject(this, &ThisClass::TravelToInGame);

	LobbyDirector->StartLobbyFlow();
	PlayerReadyCheck->StartTracking();
}

void ALobbyGameMode::HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem)
{
	LobbyDirector->NotifyItemEquipped(EquippedPlayer, EquippedItem);
}

void ALobbyGameMode::HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem)
{
}

void ALobbyGameMode::Logout(AController* ExitedPlayer)
{
	Super::Logout(ExitedPlayer);

	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	const UGameStateSubsystem* GameStateSubsystem = GameInstance ? GameInstance->GetSubsystem<UGameStateSubsystem>() : nullptr;
	if (!GameStateSubsystem)
	{
		return;
	}

	// 로비가 리셋(트래블)되기 전까지 진행 중이던 연출 타이머가 발화하지 않도록 중단
	LobbyDirector->AbortLobbyFlow();

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (const APlayerController* PC = It->Get())
		{
			if (ADefaultPlayerState* PS = PC->GetPlayerState<ADefaultPlayerState>())
			{
				PS->EquippedWeaponClass = nullptr;

				// 아래에서 바로 트래블한다. 지금 보내지 않으면 이 변경은 전송되지 못한다
				PS->ForceNetUpdate();
			}
		}
	}

	// AGameModeBase::GetNumPlayers()는 PlayerControllerList를 순회하는데,
	// RemoveController()는 Logout() 완료 후에 호출되므로 이탈 플레이어가 아직 포함됨 → -1 보정
	const int32 RemainingPlayers = GetNumPlayers() - 1;
	if (RemainingPlayers <= 0)
	{
		return;
	}

	if (RemainingPlayers < 2)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Player left in lobby. Returning to Main Menu."));
		
		const FString MainMenuMapName = GameStateSubsystem->GetLevelStringFromTag(TromboneGamePlayTags::Trombone_Maps_OutGame_MainMenu);
		UEasyStatics::ServerTravelToLevel(this, MainMenuMapName);
	}
	else
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Player left in lobby. Restarting lobby"));

		const FGameplayTag InGameTag = UEasyStatics::GetCurrentInGameMap(this, UTromboneConfig::Get()->DefaultInGameMap);
		const FGameplayTag LobbyCategory = FGameplayTag::RequestGameplayTag(FName(*TromboneGamePlayTags::LobbyPath), false);
		const FGameplayTag LobbyTag = UTromboneFunctionLibrary::GetSiblingMapTag(InGameTag, LobbyCategory);
		const FString LobbyMapName = GameStateSubsystem->GetLevelStringFromTag(LobbyTag);
		UEasyStatics::ServerTravelToLevel(this, LobbyMapName);
	}
}

void ALobbyGameMode::TravelToInGame()
{
	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	const UGameStateSubsystem* GameStateSubsystem = GameInstance ? GameInstance->GetSubsystem<UGameStateSubsystem>() : nullptr;
	if (!GameStateSubsystem)
	{
		return;
	}

	const FGameplayTag InGameTag = UEasyStatics::GetCurrentInGameMap(this, UTromboneConfig::Get()->DefaultInGameMap);
	const FString InGameMapName = GameStateSubsystem->GetLevelStringFromTag(InGameTag);
	UEasyStatics::ServerTravelToLevel(this, InGameMapName);
}
