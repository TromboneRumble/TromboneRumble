// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/GameStateSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"

static bool TryGetGameStateFromMapTag(const FGameplayTag& MapTag, EGameState& OutState)
{
	// 자식 태그가 있으면 안 됨
	const FGameplayTagContainer Children = UGameplayTagsManager::Get().RequestGameplayTagChildren(MapTag);
	if (Children.Num() > 0) return false;

	// "Trombone.Maps.InGame.Main"
	const FString TagStr = MapTag.ToString();
	TArray<FString> Parts;
	TagStr.ParseIntoArray(Parts, TEXT("."), true);

	// "Trombone.Maps.<State>.<MapName>" 형태만 통과
	if (Parts.Num() < 4)
	{
		// Debug::Print(FString::Printf(TEXT("[MapTag] Skip (Need 4 tokens): %s"), *TagStr));
		return false;
	}

	if (Parts[0] != TEXT("Trombone") || Parts[1] != TEXT("Maps"))
	{
		// Debug::Print(FString::Printf(TEXT("[MapTag] Skip (Not Trombone.Maps): %s"), *TagStr));
		return false;
	}

	const FString& StateStr = Parts[2]; // "InGame", "Lobby", "MainMenu"

	// enum 이름과 동일하면 자동 변환 가능
	const UEnum* Enum = StaticEnum<EGameState>();
	const int64 Value = Enum ? Enum->GetValueByNameString(StateStr) : INDEX_NONE;
	if (Value == INDEX_NONE)
	{
		// Debug::Print(FString::Printf(TEXT("[MapTag] Skip (No EGameState match): %s"), *StateStr));
		return false;
	}

	OutState = static_cast<EGameState>(Value);
	return true;
}

void UGameStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentGameState = EGameState::MainMenu;

	// 루트 태그
	const FGameplayTag MapsRoot = FGameplayTag::RequestGameplayTag(TEXT("Trombone.Maps"));

	// 하위 태그 전부 수집
	const FGameplayTagContainer Children =
		UGameplayTagsManager::Get().RequestGameplayTagChildren(MapsRoot);

	for (const FGameplayTag& Tag : Children)
	{
		EGameState State;
		if (!TryGetGameStateFromMapTag(Tag, State))
		{
			continue;
		}

		AddMapPathFromGameTag(Tag, State);
	}

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