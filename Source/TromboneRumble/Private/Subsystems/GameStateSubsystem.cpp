// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/GameStateSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"

void UGameStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentGameState = EGameState::MainMenu;

	
	AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_MainMenuMap, EGameState::MainMenu);
	AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_InGame_Main, EGameState::InGame);
	AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_LobbyMap, EGameState::Lobby);
	AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_InGame_Main, EGameState::InGame);
	AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_InGame_MK, EGameState::InGame);
	//AddMapPathFromGameTag(TromboneGamePlayTags::Trombone_Maps_InGame_HW, EGameState::InGame);

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

	const FString& LoadedMapName = InLoadedWorld->GetMapName();

	for (const TPair<FGameplayTag, FString>& Pair : MapTagToMapNameMap)
	{
		const FString& CachedMapName = Pair.Value;
		if (!CachedMapName.IsEmpty() && LoadedMapName.Contains(CachedMapName))
		{
			if (const EGameState* FoundState = MapTagToGameStateMap.Find(Pair.Key))
			{
				SetGameState(*FoundState);
				return;
			}
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
	if (!InTag.IsValid())
	{
		return;
	}

	const FString MapPath = UTromboneFunctionLibrary::GetMapPathByTag(InTag);
	const FString CachedMapName = FPackageName::GetShortName(MapPath);

	if (!CachedMapName.IsEmpty())
	{
		MapTagToMapNameMap.Add(InTag, CachedMapName);
		MapTagToGameStateMap.Add(InTag, InGameState);
	}

}

FString UGameStateSubsystem::GetMapNameForTag(const FGameplayTag& MapTag) const
{
	if (const FString* FoundName = MapTagToMapNameMap.Find(MapTag))
	{
		return *FoundName;
	}
	return FString();
}