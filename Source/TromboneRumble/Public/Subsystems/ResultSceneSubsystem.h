// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Framework/DefaultPlayerState.h"
#include "Data/CustomizationSaveData.h"
#include "ResultSceneSubsystem.generated.h"

USTRUCT(Blueprintable)
struct FPlayerResultSceneData
{
    GENERATED_BODY()

    FString Nickname = FString();

    float Score = 0.0f;

    FRumbleScoreData SpecificScoreData = FRumbleScoreData();

    FLinearColor PlayerSkinColor = FLinearColor::Black;

    /** 포디움에 적용할 커스터마이징(Antenna/Face/Costume) */
    FCustomizationSaveData Customization = FCustomizationSaveData();

    bool bIsLocalPlayerData = false;

    // Descending
    bool operator<(const FPlayerResultSceneData& Other) const
    {
        if (FMath::IsNearlyEqual(Score, Other.Score))
        {
            return Nickname < Other.Nickname;
        }
        return Score > Other.Score;
    }
};

/** UResultSceneSubsystem
 * 
 * 결과 레벨은 세션 파괴 후 각자 로컬(standalone)로 열리므로, 이 GameInstanceSubsystem에 결과 스냅샷을 보관.
 */
UCLASS()
class TROMBONERUMBLE_API UResultSceneSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** Save in-game data when travel to the result. 결과 맵 선택을 위해 현재 스테이지도 함께 기록 */
	void SaveResultSceneData();

	/** @return My(Local) result scene data (in-game score data) */
	const FPlayerResultSceneData& GetLocalPlayerResultSceneData();

	/** @return My(Local) rhythm rank */
	int32 GetLocalPlayerRank();

	const TArray<FPlayerResultSceneData>& GetResultSceneData() const { return CachedResultSceneData; }

	/** Open result level that matching to in-game level. Must be called after DestroySession */
	void OpenResultLevel(const UObject* WorldContextObject, bool bAbsolute = true) const;

	/** @return Result level tag that matching to in-game level. fallback: Result.OrchestraStage */
	FGameplayTag ResolveResultMapTag() const;

#if !UE_BUILD_SHIPPING
	/** Fills the snapshot with fake players so the result scene can be checked without playing a match.
	 *  @param StageName Leaf of the stage to show, such as "OrchestraStage" or "SnowField". */
	void SetDebugResultSceneData(int32 PlayerCount, const FString& StageName);
#endif

private:

	/** Result data snapshot */
	UPROPERTY(Transient)
	TArray<FPlayerResultSceneData> CachedResultSceneData;

	/** Last played level name. */
	FString LastPlayedLevelString;
};
