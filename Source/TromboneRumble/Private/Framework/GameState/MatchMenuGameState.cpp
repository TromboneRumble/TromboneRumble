#include "Framework/GameState/MatchMenuGameState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/DebugHelper.h"

AMatchMenuGameState::AMatchMenuGameState()
{
	LocalPlayerStartTag = TEXT("Local");
	PlayerStartMappings = TMap<APlayerStart*, APawn*>();
}

void AMatchMenuGameState::UpdatePlayerList()
{
	if (!HasAuthority()) return;

	TArray<FString> NewPlayerList;
	for (const APlayerState* PlayerState : PlayerArray)
	{
		if (PlayerState)
		{
			NewPlayerList.Add(PlayerState->GetPlayerName());
		}
	}
}

void AMatchMenuGameState::HandleLobbyPawnCreated(APawn* PlayerPawn)
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
		
		OnPlayerStartOccupancyChanged.Broadcast(PlayerStart, PlayerPawn);
	}
}

void AMatchMenuGameState::HandleLobbyPawnPreDestroyed(APawn* PlayerPawn)
{
	if (!PlayerPawn)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("PlayerPawn is nullptr"));
		return;
	}

	for (const auto PlayerStartMapping : PlayerStartMappings)
	{
		if (PlayerStartMapping.Value == PlayerPawn)
		{
			APlayerStart* ClearedStart = PlayerStartMapping.Key;
			PlayerStartMappings.Remove(ClearedStart);
			
			OnPlayerStartOccupancyChanged.Broadcast(ClearedStart, nullptr);
			break;
		}
	}
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
}

void AMatchMenuGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentMatchType);
}

void AMatchMenuGameState::SetMatchType(const EMatchType NewType)
{
	if (!HasAuthority()) return;

	CurrentMatchType = NewType;
	OnRep_CurrentMatchType();
}
