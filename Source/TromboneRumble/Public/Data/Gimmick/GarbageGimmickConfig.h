// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "GarbageGimmickConfig.generated.h"

class AGarbageBase;

UENUM(BlueprintType)
enum class EGarbageTargetingMode : uint8
{
	Random UMETA(DisplayName = "Random Player"),
	ScoreWeighted UMETA(DisplayName = "Score Weighted (Higher Rank = Higher Chance)")
};

/**
 * Settings of AGarbageSpawner.
 * The knockback of each garbage is not here. It belongs to the garbage Blueprint and is the same in every level.
 *
 * @see AGarbageSpawner
 */
UCLASS(meta = (DisplayName = "관중 투척"))
class TROMBONERUMBLE_API UGarbageGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UGarbageGimmickConfig()
	{
		GimmickType = EGimmickType::Trash;
		SpawnInterval.Min = 8.f;
		SpawnInterval.Max = 15.f;
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	/** Garbage thrown in this level. One is picked at random per throw. */
	UPROPERTY(EditAnywhere, Category = "Garbage", meta = (DisplayName = "투척물 종류"))
	TArray<TSubclassOf<AGarbageBase>> GarbageClasses;

	/** Wait between two throws. */
	UPROPERTY(EditAnywhere, Category = "Garbage", meta = (DisplayName = "투척 간격"))
	FGimmickInterval SpawnInterval;

	/** The landing point is moved by up to this many cm from the target player. */
	UPROPERTY(EditAnywhere, Category = "Garbage", meta = (DisplayName = "타겟팅 오차 반경", ClampMin = "0.0", Units = "cm"))
	float TargetRandomRadius = 80.f;

	/** Who gets targeted. Score weighted picks higher ranked players more often. */
	UPROPERTY(EditAnywhere, Category = "Garbage", meta = (DisplayName = "대상 타겟팅 방식"))
	EGarbageTargetingMode TargetingMode = EGarbageTargetingMode::ScoreWeighted;
};
