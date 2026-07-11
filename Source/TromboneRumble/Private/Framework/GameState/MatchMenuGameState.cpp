// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Framework/GameState/MatchMenuGameState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Net/UnrealNetwork.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Utilities/DebugHelper.h"

AMatchMenuGameState::AMatchMenuGameState()
{
	LocalPlayerStartTag = TEXT("Local");
	PlayerStartMappings = TMap<APlayerStart*, APawn*>();
	SelectedLobbyMapTag = TromboneGamePlayTags::Trombone_Maps_Lobby_OrchestraStage;
}

void AMatchMenuGameState::HandleMatchPawnCreated(APawn* PlayerPawn)
{
	if (!PlayerPawn)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("PlayerPawn is nullptr"));
		return;
	}

	if (PlayerPawn->IsReplicatingMovement())
	{
		const FString DebugMsg = FString::Printf(TEXT("could not relocate pawn '%s' because it has movement replication enabled! Disable bReplicateMovement to fix."), *PlayerPawn->GetActorNameOrLabel());
		LOG_WITH_CURRENT_CONTEXT(Error, DebugMsg);
		return;
	}

	if (APlayerStart* PlayerStart = FindPlayerStart(PlayerPawn))
	{
		PlayerStartMappings[PlayerStart] = PlayerPawn;
		PlayerPawn->TeleportTo(PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation());
		
		OnPlayerStartOccupancyChangedEvent.Broadcast(PlayerStart, PlayerPawn);
	}
	
	OnPlayerCountChangedEvent.Broadcast(GetCurrentPlayerCount());
}

void AMatchMenuGameState::HandleMatchPawnPreDestroyed(APawn* PlayerPawn)
{
	if (!PlayerPawn)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("PlayerPawn is nullptr"));
		return;
	}
	
	for (auto& Pair : PlayerStartMappings)
	{
		if (Pair.Value == PlayerPawn)
		{
			APlayerStart* ClearedStart = Pair.Key;
			Pair.Value = nullptr; 
          
			OnPlayerStartOccupancyChangedEvent.Broadcast(ClearedStart, nullptr);
			break;
		}
	}
	
	OnPlayerCountChangedEvent.Broadcast(GetCurrentPlayerCount());
}

APlayerStart* AMatchMenuGameState::FindPlayerStart(APawn* PlayerPawn) const
{
	if (!PlayerPawn)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("PlayerPawn is nullptr"));
		return nullptr;
	}

	for (auto PossiblePlayerStart : PlayerStartMappings)
	{
		if (PossiblePlayerStart.Value != nullptr)
		{
			continue;
		}

		APlayerStart* PlayerStartActor = PossiblePlayerStart.Key;
		if (!PlayerStartActor)
		{
			continue;
		}

		// Match player start and player pawn type (local / remote).
		if ((PlayerStartActor->PlayerStartTag == LocalPlayerStartTag) == PlayerPawn->IsLocallyControlled())
		{
			return PlayerStartActor;
		}
	}

	const FString DebugMsg = FString::Printf(TEXT("No empty PlayerStart found for pawn %s"), *PlayerPawn->GetActorNameOrLabel());
	LOG_WITH_CURRENT_CONTEXT(Warning, DebugMsg);
	return nullptr;
}

void AMatchMenuGameState::OnRep_CurrentMatchType()
{
	OnMatchTypeChanged.Broadcast(CurrentMatchType);
}

void AMatchMenuGameState::OnRep_SelectedLobbyMap()
{
	OnSelectedMapChanged.Broadcast(SelectedLobbyMapTag);
}

void AMatchMenuGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// Do nothing in the editor (e.g. blueprint editor).
	UWorld* MyWorld = GetWorld();
	if (MyWorld && !MyWorld->IsGameWorld())
	{
		return;
	}

	for (TActorIterator<APlayerStart> ActorItr(MyWorld); ActorItr; ++ActorItr)
	{
		if (APlayerStart* PlayerStart = *ActorItr)
		{
			PlayerStartMappings.Add(PlayerStart, nullptr);
		}
	}
	
	if (HasAuthority())
	{
		if (const UTromboneConfig* Config = UTromboneConfig::Get())
		{
			const FGameplayTag LobbyCategory = FGameplayTag::RequestGameplayTag(FName(*TromboneGamePlayTags::LobbyPath), false);
			const FGameplayTag DefaultLobby = UTromboneFunctionLibrary::GetSiblingMapTag(Config->DefaultInGameMap, LobbyCategory);
			if (DefaultLobby.IsValid())
			{
				SelectedLobbyMapTag = DefaultLobby;
			}
		}
	}
}

void AMatchMenuGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentMatchType);
	DOREPLIFETIME(ThisClass, SelectedLobbyMapTag);
}

void AMatchMenuGameState::SetMatchType(const EMatchType NewType)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentMatchType = NewType;
	OnRep_CurrentMatchType();
}

void AMatchMenuGameState::SetSelectedLobbyMap(const FGameplayTag NewLobbyTag)
{
	if (!HasAuthority() || !NewLobbyTag.IsValid())
	{
		return;
	}

	SelectedLobbyMapTag = NewLobbyTag;
	OnRep_SelectedLobbyMap();
}

int32 AMatchMenuGameState::GetCurrentPlayerCount() const
{
	int32 PlayerCount = 0;
	for (const auto& Pair : PlayerStartMappings)
	{
		if (Pair.Value != nullptr)
		{
			PlayerCount++;
		}
	}

	return PlayerCount;
}
