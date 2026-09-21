// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "WaterDropGimmickConfig.generated.h"

/**
 * Settings of AWaterDropSpawner with the Puddle type (BP_WaterDropSpawner).
 *
 * @see AWaterDropSpawner
 */
UCLASS(meta = (DisplayName = "누수 물자국"))
class TROMBONERUMBLE_API UWaterDropGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UWaterDropGimmickConfig() { GimmickType = EGimmickType::Puddle; }

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	/** Seconds between two drops. 0 stops the drops. */
	UPROPERTY(EditAnywhere, Category = "WaterDrop", meta = (DisplayName = "생성 간격", ClampMin = "0.0", Units = "s"))
	float SpawnInterval = 15.f;

	/** Seconds the puddle takes to grow from nothing to its full size after the drop lands. */
	UPROPERTY(EditAnywhere, Category = "Puddle", meta = (DisplayName = "웅덩이 확장 시간", ClampMin = "0.0", Units = "s"))
	float PuddleGrowDuration = 0.5f;

	/** Seconds the puddle stays at full size before it starts to fade. */
	UPROPERTY(EditAnywhere, Category = "Puddle", meta = (DisplayName = "웅덩이 유지 시간", ClampMin = "0.0", Units = "s"))
	float PuddleFadeDelay = 10.f;

	/** Seconds the puddle takes to fade out. It is removed when this ends. */
	UPROPERTY(EditAnywhere, Category = "Puddle", meta = (DisplayName = "웅덩이 소멸 시간", ClampMin = "0.0", Units = "s"))
	float PuddleFadeDuration = 1.f;
};

/**
 * Same settings for the Ice type (BP_SnowDropSpawner). The spawner class is shared, only the gimmick type differs.
 */
UCLASS(meta = (DisplayName = "빙판"))
class TROMBONERUMBLE_API UIceDropGimmickConfig : public UWaterDropGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UIceDropGimmickConfig() { GimmickType = EGimmickType::Ice; }
};
