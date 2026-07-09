// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/PlayerReadyCheckComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/DebugHelper.h"

UPlayerReadyCheckComponent::UPlayerReadyCheckComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerReadyCheckComponent::StartTracking()
{
	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UGameStateSubsystem* GameStateSubsystem = GameInstance ? GameInstance->GetSubsystem<UGameStateSubsystem>() : nullptr;
	if (!GameStateSubsystem)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("GameStateSubsystem not found. Ready tracking disabled."));
		return;
	}

	GameStateSubsystem->OnPlayerLoadingScreenFinished.AddUObject(this, &ThisClass::HandlePlayerLoadingScreenFinished);

	// 리슨서버 호스트는 로딩 스크린 종료 통지가 바인딩 전에 지나갔을 수 있으므로 즉시 처리
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->IsLocalController())
		{
			HandlePlayerLoadingScreenFinished(PC);
		}
	}
}

void UPlayerReadyCheckComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>())
			{
				GameStateSubsystem->OnPlayerLoadingScreenFinished.RemoveAll(this);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UPlayerReadyCheckComponent::HandlePlayerLoadingScreenFinished(APlayerController* PC)
{
	if (!PC)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Invalid PlayerController"));
		return;
	}

	if (ReadyPlayers.Contains(PC))
	{
		LOG_WITH_CURRENT_CONTEXT(Log, FString::Printf(TEXT("Player %s has already been marked as ready"), *PC->GetName()));
		return;
	}

	ReadyPlayers.Add(PC);
	OnPlayerReady.Broadcast(PC);

	if (!bAllReadyBroadcasted && ReadyPlayers.Num() >= GetExpectedPlayerCount())
	{
		bAllReadyBroadcasted = true;
		OnAllPlayersReady.Broadcast();
	}
}

int32 UPlayerReadyCheckComponent::GetExpectedPlayerCount() const
{
	AGameModeBase* OwnerGameMode = Cast<AGameModeBase>(GetOwner());
	return OwnerGameMode ? OwnerGameMode->GetNumPlayers() : 0;
}
