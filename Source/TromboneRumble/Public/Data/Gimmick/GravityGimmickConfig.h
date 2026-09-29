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

	/** 예고 중 경고등이 1초에 밝아졌다 어두워지는 횟수. 클수록 빠르게 깜빡인다 (Hz) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "경고등 깜빡임 속도", ClampMin = "0.1", Units = "Hz"))
	float WarningLightFrequency = 2.f;

	/** 경고등이 가장 어두울 때의 밝기. 레벨에 배치한 밝기를 1로 본 비율이며, 0이면 완전히 꺼졌다 켜진다 */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "경고등 최소 밝기", ClampMin = "0.0", ClampMax = "1.0"))
	float WarningLightMinRatio = 0.2f;
};
