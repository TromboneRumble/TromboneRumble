// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/GameStateSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"

void UGameStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentGameState = EGameState::MainMenu;

	
	AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_MainMap, EGameState::MainMenu);
	AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_InGameMap, EGameState::InGame);
	AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_LobbyMap, EGameState::Lobby);

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

void UGameStateSubsystem::OnPostLoadMap(UWorld* InLoadedWorld)
{
	if (!InLoadedWorld)
	{
		SetGameState(EGameState::Invalid);
		return;
	}

	const FString& MapName = InLoadedWorld->GetMapName();
	for (const TPair<EGameState, FString>& Pair : GameStateToMapNameMap)
	{
		if (!Pair.Value.IsEmpty() && MapName.Contains(Pair.Value))
		{
			SetGameState(Pair.Key);
			return;
		}
	}
	// 어떤 것도 매칭 안 되면 Invalid
	SetGameState(EGameState::Invalid);
}

void UGameStateSubsystem::SetGameState(const EGameState& InNewState)
{
	if (CurrentGameState != InNewState)
	{
		CurrentGameState = InNewState;
		OnGameStateChanged.Broadcast(CurrentGameState);
	}
}

void UGameStateSubsystem::AddMapPathFromGameTag(const FGameplayTag& InTag, const EGameState& InGameState)
{
	FString CachedMapName = FPackageName::GetShortName(
		UTromboneFunctionLibrary::GetMapPathByTag(InTag)
	);
	if (!CachedMapName.IsEmpty())
	{
		GameStateToMapNameMap.Add(InGameState,CachedMapName);
	}

}

FString UGameStateSubsystem::GetMapNameForGameState(const EGameState& InGameState) const
{
	if (const FString* FoundName = GameStateToMapNameMap.Find(InGameState))
	{
		return *FoundName;
	}
	return FString();
}
