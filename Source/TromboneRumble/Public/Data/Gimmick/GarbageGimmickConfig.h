// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "GarbageGimmickConfig.generated.h"

class AGarbageBase;

UENUM(BlueprintType)
enum class EGarbageTargetingMode : uint8
{
	Random UMETA(DisplayName = "무작위 플레이어"),
	ScoreWeighted UMETA(DisplayName = "순위 가중 (순위가 높을수록 자주)")
};

/**
 * Settings of AGarbageSpawner.
 * The knockback of each garbage is not here. It belongs to the garbage Blueprint and is the same in every level.
 *
 * @see AGarbageSpawner
 */
UCLASS(meta = (DisplayName = "관중 투척"))
class TROMBONERUMBLE_API UGarbageGimmickConfig : public USpawnerGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UGarbageGimmickConfig()
	{
		GimmickType = EGimmickType::Garbage;
		FirstSpawnDelay = 8.f;
		Normal.Interval.Min = 8.f;
		Normal.Interval.Max = 15.f;
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	//~ End UGimmickConfig Interface

	//~ Begin USpawnerGimmickConfig Interface
	virtual float BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, float SpawnTime) const override;
	//~ End USpawnerGimmickConfig Interface
#endif

	/** 이 레벨에서 관중이 던지는 투척물 종류. 던질 때마다 하나를 무작위로 고른다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "투척물 종류"))
	TArray<TSubclassOf<AGarbageBase>> GarbageClasses;

	/** 착지 지점이 타겟 플레이어에게서 최대 이만큼 벗어난다 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "타겟팅 오차 반경", ClampMin = "0.0", Units = "cm"))
	float TargetRandomRadius = 80.f;

	/** 누구를 노릴지 정한다. 순위 가중은 순위가 높은 플레이어를 더 자주 노린다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "대상 타겟팅 방식"))
	EGarbageTargetingMode TargetingMode = EGarbageTargetingMode::ScoreWeighted;
};
