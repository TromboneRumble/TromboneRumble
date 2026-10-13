// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "GravityGimmickConfig.generated.h"

/**
 * Settings of AGravityGimmick.
 *
 * @see AGravityGimmick
 */
UCLASS(meta = (DisplayName = "중력", GroupSettingsByCategory))
class TROMBONERUMBLE_API UGravityGimmickConfig : public UEventGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UGravityGimmickConfig()
	{
		GimmickType = EGimmickType::Gravity;
		bSpawnableByManager = true;
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual float BuildEventTimeline(FGimmickTimelineBuilder& Builder, float Start) const override;
	//~ End UGimmickConfig Interface
#endif

	/** 발동 후 중력이 바뀐 채로 유지되는 시간. 끝나면 원래 중력으로 돌아온다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "지속 시간", ClampMin = "0.1", Units = "s"))
	float ActiveDuration = 10.f;

	/** 발동 중 플레이어 중력에 곱하는 값. 1보다 작으면 둥둥 뜨고, 1보다 크면 무거워진다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "중력 배율", ClampMin = "0.05", ClampMax = "20.0"))
	float GravityMultiplier = 0.3f;

	/** 끝날 때 먹먹한 소리가 원래대로 돌아오기까지 걸리는 시간. 일정한 속도로 돌아온다 (초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "먹먹함이 풀리는 시간", ClampMin = "0.0", Units = "s"))
	float MuffleOutDuration = 1.f;
};
