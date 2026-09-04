// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "TromboneGamePlayTags.h"
#include "DeveloperSettings/GameMapDeveloperSettings.h"
#include "GameFramework/Actor.h"

FString UTromboneFunctionLibrary::GetMapPathByMapTag(const FGameplayTag InMapTag)
{
    const UGameMapDeveloperSettings* Settings = GetDefault<UGameMapDeveloperSettings>();
    checkf(Settings, TEXT("GameMapDeveloperSettings is null"));

    const FSoftObjectPath* Found = Settings->GamePlayMap.Find(InMapTag);
    checkf(Found, TEXT("No map path for tag: %s"), *InMapTag.ToString());

    return Found->GetLongPackageName();
}

TArray<FGameplayTag> UTromboneFunctionLibrary::GetMapTagsUnderCategory(const FGameplayTag CategoryTag)
{
    TArray<FGameplayTag> Result;
    if (!CategoryTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid category tag: %s"), *CategoryTag.ToString());
        return Result;
    }

    const UGameMapDeveloperSettings* Settings = GetDefault<UGameMapDeveloperSettings>();
    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load GameMapDeveloperSettings"));
        return Result;
    }

    for (const TPair<FGameplayTag, FSoftObjectPath>& Pair : Settings->GamePlayMap)
    {
        if (Pair.Key.RequestDirectParent() != CategoryTag)
        {
            continue;
        }

#if UE_BUILD_SHIPPING
        if (Settings->ShippingHiddenMaps.Contains(Pair.Key))
        {
            continue;
        }
#endif

        Result.Add(Pair.Key);
    }

    Result.Sort([](const FGameplayTag& A, const FGameplayTag& B)
    {
        return A.ToString() < B.ToString();
    });

    return Result;
}

FGameplayTag UTromboneFunctionLibrary::GetSiblingMapTag(const FGameplayTag SourceTag, const FGameplayTag TargetCategoryTag)
{
    if (!SourceTag.IsValid() || !TargetCategoryTag.IsValid())
    {
        return FGameplayTag();
    }

    // SourceTag의 leaf(테마) 추출. 예: "Trombone.Maps.Lobby.OrchestraStage" → "OrchestraStage"
    const FString SourceStr = SourceTag.ToString();
    FString Leaf;
    if (!SourceStr.Split(TEXT("."), nullptr, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        return FGameplayTag();
    }

    // TargetCategory.Leaf 조합. 예: "Trombone.Maps.InGame" + "." + "OrchestraStage"
    const FString TargetStr = TargetCategoryTag.ToString() + TEXT(".") + Leaf;
    return FGameplayTag::RequestGameplayTag(FName(*TargetStr), false);
}

FGameplayTag UTromboneFunctionLibrary::LobbyToInGameTag(const FGameplayTag LobbyTag)
{
    const FGameplayTag InGameCategory = FGameplayTag::RequestGameplayTag(FName(*(TromboneGamePlayTags::InGamePath)), false);
    return GetSiblingMapTag(LobbyTag, InGameCategory);
}

FGameplayTag UTromboneFunctionLibrary::PickRandomSongForMap(const FGameplayTag InGameMapTag)
{
    if (InGameMapTag == TromboneGamePlayTags::Trombone_Maps_InGame_SnowField)
    {
        return FMath::RandBool() ? TromboneGamePlayTags::Trombone_Rhythm_Song_MapC
                                 : TromboneGamePlayTags::Trombone_Rhythm_Song_MapD;
    }

    // OrchestraStage 및 그 외 폴백
    return FMath::RandBool() ? TromboneGamePlayTags::Trombone_Rhythm_Song_EasyMapA
                             : TromboneGamePlayTags::Trombone_Rhythm_Song_EasyMapB;
}

void UTromboneFunctionLibrary::PrintDebug(const FString& Msg, const int32 InKey, const FLinearColor Color, const float Duration,
                                          const bool bRandomColor, const bool bLog)
{
#if !UE_BUILD_SHIPPING
	FColor FinalColor = bRandomColor ? FColor::MakeRandomColor() : Color.ToFColor(true);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(InKey, Duration, FinalColor, Msg);
	}

	// 로그 출력
	if (bLog)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
	}
#endif
}
