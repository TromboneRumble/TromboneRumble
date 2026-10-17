// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "WaterDropGimmickConfig.generated.h"

/**
 * Settings of AWaterDropSpawner with the WaterDrop type (BP_WaterDropSpawner).
 *
 * @see AWaterDropSpawner
 */
UCLASS(meta = (DisplayName = "누수 물자국"))
class TROMBONERUMBLE_API UWaterDropGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UWaterDropGimmickConfig() { GimmickType = EGimmickType::WaterDrop; }

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	/** 물방울 하나가 떨어진 뒤 다음 물방울까지의 시간. 0이면 떨어지지 않는다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "생성 간격", ClampMin = "0.0", Units = "s"))
	float SpawnInterval = 15.f;

	/** 물방울이 떨어진 뒤 바닥 자국(누수는 물, 빙판은 얼음)이 다 커지기까지의 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "커지는 시간", ClampMin = "0.0", Units = "s"))
	float GrowDuration = 0.5f;

	/** 자국이 다 커진 뒤 사라지기 시작할 때까지 그대로 남는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "유지 시간", ClampMin = "0.1", Units = "s"))
	float HoldDuration = 10.f;

	/** 자국이 서서히 사라지는 시간. 끝나면 제거된다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "사라지는 시간", ClampMin = "0.0", Units = "s"))
	float FadeDuration = 1.f;
};

/**
 * Same settings for the IceDrop type (BP_SnowDropSpawner). The spawner class is shared, only the gimmick type differs.
 */
UCLASS(meta = (DisplayName = "빙판"))
class TROMBONERUMBLE_API UIceDropGimmickConfig : public UWaterDropGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UIceDropGimmickConfig() { GimmickType = EGimmickType::IceDrop; }
};
