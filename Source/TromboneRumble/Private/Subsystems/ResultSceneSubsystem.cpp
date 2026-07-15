// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/ResultSceneSubsystem.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Subsystems/GameStateSubsystem.h"

void UResultSceneSubsystem::SaveResultSceneData()
{
	CachedResultSceneData.Empty();

	// 결과 레벨 선택용으로 현재 레벨을 기록
	LastPlayedLevelString.Empty();
	if (const UGameStateSubsystem* GameStateSubsystem = GetGameInstance()->GetSubsystem<UGameStateSubsystem>())
	{
		if (const UEnum* Enum = StaticEnum<ELevelType>())
		{
			LastPlayedLevelString = Enum->GetNameStringByValue(static_cast<int64>(GameStateSubsystem->GetLevelState()));
		}
	}

	const APlayerState* LocalPS = GetWorld()->GetFirstPlayerController()->GetPlayerState<APlayerState>();
	if (LocalPS == nullptr)
	{
		return;
	}

	for (APlayerState* PS : TActorRange<APlayerState>(GetWorld()))
	{
		if (const ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(PS))
		{
			FPlayerResultSceneData Data;
			Data.Nickname = DPS->GetPlayerName();
			Data.Score = DPS->GetScore();
			Data.SpecificScoreData = DPS->GetScoreData();
			Data.PlayerSkinColor = DPS->GetSkinColor();
			Data.Customization = DPS->GetCustomizationData();
			Data.bIsLocalPlayerData = (DPS == LocalPS);

			CachedResultSceneData.Add(Data);
		}
	}
}

const FPlayerResultSceneData& UResultSceneSubsystem::GetLocalPlayerResultSceneData()
{
	for (const auto& SingleData : CachedResultSceneData)
	{
		if (SingleData.bIsLocalPlayerData)
		{
			return SingleData;
		}
	}

	// If there is no local player data, create and return it
	// this situation should not actually occur
	CachedResultSceneData.Add(FPlayerResultSceneData());
	return CachedResultSceneData.Last();
}

int32 UResultSceneSubsystem::GetLocalPlayerRank()
{
	CachedResultSceneData.Sort();

	for (int Rank = 0; Rank < CachedResultSceneData.Num(); Rank++)
	{
		if (CachedResultSceneData[Rank].bIsLocalPlayerData)
		{
			return Rank + 1;
		}
	}

	return -1;
}

FGameplayTag UResultSceneSubsystem::ResolveResultMapTag() const
{
	const FGameplayTag FallbackTag = TromboneGamePlayTags::Trombone_Maps_Result_OrchestraStage;
	if (LastPlayedLevelString.IsEmpty())
	{
		return FallbackTag;
	}

	const FString CandidateStr = TromboneGamePlayTags::ResultPath + TEXT(".") + LastPlayedLevelString;
	const FGameplayTag CandidateTag = FGameplayTag::RequestGameplayTag(FName(*CandidateStr), false);
	if (!CandidateTag.IsValid())
	{
		return FallbackTag;
	}

	const FGameplayTag ResultCategoryTag = FGameplayTag::RequestGameplayTag(FName(*TromboneGamePlayTags::ResultPath), false);
	if (!UTromboneFunctionLibrary::GetMapTagsUnderCategory(ResultCategoryTag).Contains(CandidateTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UResultSceneSubsystem::ResolveResultMapTag] No result map registered for stage '%s'. Falling back to %s"), *LastPlayedLevelString, *FallbackTag.ToString());
		return FallbackTag;
	}

	return CandidateTag;
}

void UResultSceneSubsystem::OpenResultLevel(const UObject* WorldContextObject, const bool bAbsolute) const
{
	const FString MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(ResolveResultMapTag());
	const FString URL = FPackageName::ObjectPathToPackageName(MapPath);
	UGameplayStatics::OpenLevel(WorldContextObject, FName(*URL), bAbsolute);
}
