// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/GameStateSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"

void UGameStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentGameState = EGameState::MainMenu;

	CachedLobbyMapName = FPackageName::GetShortName(
		UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_LobbyMap)
	);
	CachedInGameMapName = FPackageName::GetShortName(
		UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_InGameMap)
	);
	CachedMainMenuMapName = FPackageName::GetShortName(
		UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMap)
	);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UGameStateSubsystem::OnPostLoadMap);
	
	OnPostLoadMap(GetWorld());
}

void UGameStateSubsystem::Deinitialize()
{
	if (FCoreUObjectDelegates::PostLoadMapWithWorld.IsBoundToObject(this))
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	}
	
	Super::Deinitialize();
}

void UGameStateSubsystem::OnPostLoadMap(UWorld* LoadedWorld)
{
	const FString& MapName = LoadedWorld->GetMapName();
	if (MapName.Contains(CachedInGameMapName))
	{
		SetGameState(EGameState::InGame);
	}
	else if (MapName.Contains(CachedLobbyMapName))
	{
		SetGameState(EGameState::Lobby);
	}
	else if (MapName.Contains(CachedMainMenuMapName))
	{
		SetGameState(EGameState::MainMenu);
	}
	else
	{
		SetGameState(EGameState::Invalid); 
	}
}

void UGameStateSubsystem::SetGameState(const EGameState NewState)
{
	if (CurrentGameState != NewState)
	{
		CurrentGameState = NewState;
		OnGameStateChanged.Broadcast(CurrentGameState);
	}
}
