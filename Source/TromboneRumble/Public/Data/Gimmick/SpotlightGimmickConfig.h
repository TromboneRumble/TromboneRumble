// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "SpotlightGimmickConfig.generated.h"

/**
 * Settings of ASpotlightManager. Each spawn makes spotlight zones, and each zone warns, turns on, then fades.
 * ASpotlightZone runs the phases of GetZonePhases, and the timeline draws the same phases.
 *
 * @see ASpotlightManager
 */
UCLASS(meta = (DisplayName = "스포트라이트"))
class TROMBONERUMBLE_API USpotlightGimmickConfig : public USpawnerGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	USpotlightGimmickConfig()
	{
		GimmickType = EGimmickType::Spotlight;

		Normal.Interval.Min = 8.f;
		Normal.Interval.Max = 15.f;
		Normal.MinSpawnCount = 1;
		Normal.MaxSpawnCount = 2;

		Fever.Interval.Min = 5.f;
		Fever.Interval.Max = 8.f;
		Fever.MinSpawnCount = 2;
		Fever.MaxSpawnCount = 4;
		bUseFeverRule = true;
	}

#if WITH_EDITOR
	//~ Begin USpawnerGimmickConfig Interface
	virtual float BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, float SpawnTime) const override;
	//~ End USpawnerGimmickConfig Interface
#endif

	/**
	 * Add the phases one spotlight zone goes through, in order.
	 *
	 * @param bFever Whether the zone was made in fever time.
	 * @param OutPhases The phases are added here.
	 */
	void GetZonePhases(bool bFever, TArray<FGimmickPhase>& OutPhases) const;

	/** 스포트라이트가 생긴 뒤 켜지기까지의 시간. 플레이어는 이 동안 안으로 들어간다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "예고 시간", ClampMin = "0.0", Units = "s"))
	float WarningDuration = 1.5f;

	/** 켜면 피버 타임에 생긴 스포트라이트는 예고 없이 바로 켜진다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "피버 타임 시 예고 생략"))
	bool bSkipWarningInFever = true;

	/** 스포트라이트가 켜져 있는 시간. 이 동안 안에서 연주에 성공하면 점수를 얻는다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "발동 시간", ClampMin = "0.0", Units = "s"))
	float ActiveDuration = 3.f;

	/** 켜진 시간이 끝난 뒤 스포트라이트가 서서히 사라지는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "사라지는 시간", ClampMin = "0.0", Units = "s"))
	float FadeDuration = 2.f;

	/** 스포트라이트 안에서 연주에 성공하면 얻는 점수 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|물체", meta = (DisplayName = "성공 보너스 점수", ClampMin = "0"))
	int32 BonusScore = 70;
};
