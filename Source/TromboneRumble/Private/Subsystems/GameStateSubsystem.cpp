// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/GameStateSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"

static bool TryGetGameStateFromMapTag(const FGameplayTag& MapTag, ELevelType& OutState)
{
	// 자식 태그가 있으면 안 됨
	const FGameplayTagContainer Children = UGameplayTagsManager::Get().RequestGameplayTagChildren(MapTag);
	if (Children.Num() > 0)
	{
		return false;
	}

	const FString TagStr = MapTag.ToString();
	TArray<FString> Parts;
	TagStr.ParseIntoArray(Parts, TEXT("."), true);

	// Trombone.Maps.<Category>.<Leaf> 형태만 통과
	if (Parts.Num() < 4 ||
		Parts[0] != *TromboneGamePlayTags::ProjectName ||
		Parts[1] != *TromboneGamePlayTags::MapsCategory)
	{
		return false;
	}

	const FString& Category = Parts[2]; // "InGame", "Lobby", "OutGame", "Test", ...
	const FString& Leaf     = Parts[3]; // "OrchestraStage", "SnowField", "MainMenu", ...

	// InGame.<Leaf>  → <Leaf>		(예: InGame.OrchestraStage → OrchestraStage)
	// OutGame.<Leaf> → <Leaf>		(예: OutGame.MainMenu → MainMenu)
	// Lobby.<Leaf>	 → <Leaf>Lobby	(예: Lobby.OrchestraStage → OrchestraStageLobby)
	// Result.<Leaf>  → Result		(예: Result.OrchestraStage → Result)
	// 위에 해당하지 않는 카테고리(Test 등)는 ELevelType에 없어 제외됨
	FString EnumName;
	if (Category == TromboneGamePlayTags::LobbyCategory)
	{
		EnumName = Leaf + TromboneGamePlayTags::LobbyCategory;
	}
	else if (Category == TromboneGamePlayTags::ResultCategory)
	{
		EnumName = TromboneGamePlayTags::ResultCategory;
	}
	else
	{
		EnumName = Leaf;
	}

	const UEnum* Enum = StaticEnum<ELevelType>();
	const int64 Value = Enum ? Enum->GetValueByNameString(EnumName) : INDEX_NONE;
	if (Value == INDEX_NONE)
	{
		return false;
	}

	OutState = static_cast<ELevelType>(Value);
	return true;
}

void UGameStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentLevelType = ELevelType::MainMenu;

	// 루트 태그
	const FGameplayTag MapsRoot = FGameplayTag::RequestGameplayTag(*TromboneGamePlayTags::MapsRootPath);

	// 하위 태그 전부 수집
	const FGameplayTagContainer Children = UGameplayTagsManager::Get().RequestGameplayTagChildren(MapsRoot);

	for (const FGameplayTag& Tag : Children)
	{
		ELevelType State;
		if (!TryGetGameStateFromMapTag(Tag, State))
		{
			continue;
		}

		AddMapPathFromGameTag(Tag, State);
	}

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ThisClass::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnPostLoadMap);
	
	OnPostLoadMap(GetWorld());
}

void UGameStateSubsystem::Deinitialize()
{
	if (FCoreUObjectDelegates::PreLoadMap.IsBoundToObject(this))
	{
		FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	}
	if (FCoreUObjectDelegates::PostLoadMapWithWorld.IsBoundToObject(this))
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	}
	
	Super::Deinitialize();
}

void UGameStateSubsystem::OnPreLoadMap(const FString& InMapName)
{
	if (!UpdateLevelStateFromMapName(InMapName))
	{
		SetLevelState(ELevelType::Invalid);
	}
}

void UGameStateSubsystem::OnPostLoadMap(UWorld* InLoadedWorld)
{
	if (!InLoadedWorld)
	{
		SetLevelState(ELevelType::Invalid);
		return;
	}
	
	if (!UpdateLevelStateFromMapName(InLoadedWorld->GetMapName()))
	{
		SetLevelState(ELevelType::Invalid);
	}
}

void UGameStateSubsystem::SetLevelState(const ELevelType& InNewState)
{
	if (CurrentLevelType != InNewState)
	{
		CurrentLevelType = InNewState;
		OnLevelStateChanged.Broadcast(CurrentLevelType);
	}
}

void UGameStateSubsystem::AddMapPathFromGameTag(const FGameplayTag& InTag, const ELevelType& InLevelState)
{
	if (!InTag.IsValid())
	{
		return;
	}

	const FString MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(InTag);
	const FString MapString = FPackageName::GetShortName(MapPath);

	if (!MapString.IsEmpty())
	{
		MapTagToMapNameMap.Add(InTag, MapString);
		MapTagToLevelTypeMap.Add(InTag, InLevelState);
	}
}

bool UGameStateSubsystem::UpdateLevelStateFromMapName(const FString& InMapName)
{
	if (InMapName.IsEmpty())
	{
		return false;
	}

	const FString ShortMapName = FPackageName::GetShortName(InMapName);

	for (const TPair<FGameplayTag, FString>& Pair : MapTagToMapNameMap)
	{
		const FString& CachedMapName = Pair.Value;
       
		if (!CachedMapName.IsEmpty() && ShortMapName.Contains(CachedMapName))
		{
			if (const ELevelType* FoundState = MapTagToLevelTypeMap.Find(Pair.Key))
			{
				SetLevelState(*FoundState);
				return true;
			}
		}
	}
    
	return false;
}

FString UGameStateSubsystem::GetLevelStringFromTag(const FGameplayTag& MapTag) const
{
	if (const FString* FoundName = MapTagToMapNameMap.Find(MapTag))
	{
		return *FoundName;
	}
	return FString();
}

ELevelType UGameStateSubsystem::GetLevelState() const
{	
	if (CurrentLevelType != ELevelType::Invalid)
	{
		return CurrentLevelType;
	}

	if (const UWorld* CurrentWorld = GetWorld())
	{
		const FString LoadedMapName = CurrentWorld->GetMapName();

		for (const TPair<FGameplayTag, FString>& Pair : MapTagToMapNameMap)
		{
			const FString& CachedMapName = Pair.Value;
			if (!CachedMapName.IsEmpty() && LoadedMapName.Contains(CachedMapName))
			{
				if (const ELevelType* FoundState = MapTagToLevelTypeMap.Find(Pair.Key))
				{
					return *FoundState;
				}
			}
		}
	}

	return CurrentLevelType;
}

void UGameStateSubsystem::DumpSettings() const
{
    UE_LOG(LogTemp, Log, TEXT("=========================================================================================="));
    UE_LOG(LogTemp, Log, TEXT(" [UGameStateSubsystem::Trombone_LevelState_Dump] - Registered Maps & States"));
    UE_LOG(LogTemp, Log, TEXT("=========================================================================================="));
    
    // 1. 현재 전역 상태 정보 출력
    const UEnum* EnumPtr = StaticEnum<ELevelType>();
    FString CurrentStateStr = EnumPtr ? EnumPtr->GetNameStringByValue(static_cast<int64>(CurrentLevelType)) : TEXT("Unknown");
    
    UE_LOG(LogTemp, Log, TEXT(" * Current Level State : %s"), *CurrentStateStr);
    UE_LOG(LogTemp, Log, TEXT(" * Total Registered Maps: %d"), MapTagToMapNameMap.Num());
    UE_LOG(LogTemp, Log, TEXT("------------------------------------------------------------------------------------------"));
    
    // 테이블 헤더 출력
    UE_LOG(LogTemp, Log, TEXT(" %-40s | %-25s | %-15s"), TEXT("Gameplay Tag"), TEXT("Cached Map Name"), TEXT("Level State"));
    UE_LOG(LogTemp, Log, TEXT("------------------------------------------------------------------------------------------"));

    // 2. 루프를 돌며 맵 구조 정보 출력
    for (const TPair<FGameplayTag, FString>& Pair : MapTagToMapNameMap)
    {
        const FGameplayTag& MapTag = Pair.Key;
        const FString& CachedMapName = Pair.Value;
        
        // 매칭되는 레벨 상태 찾기
        FString StateStr = TEXT("Invalid");
        if (const ELevelType* FoundState = MapTagToLevelTypeMap.Find(MapTag))
        {
            if (EnumPtr)
            {
                StateStr = EnumPtr->GetNameStringByValue(static_cast<int64>(*FoundState));
            }
        }

        // 가로 정렬폭(%-40s 등)을 맞춰서 깔끔하게 출력
        UE_LOG(LogTemp, Log, TEXT(" %-40s | %-25s | %-15s"), 
            *MapTag.ToString(), 
            *CachedMapName, 
            *StateStr);
    }

    UE_LOG(LogTemp, Log, TEXT("=========================================================================================="));
}
