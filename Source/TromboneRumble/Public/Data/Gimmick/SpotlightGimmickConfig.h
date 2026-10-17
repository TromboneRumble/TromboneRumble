// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "SpotlightGimmickConfig.generated.h"

/** How often and how many spotlights appear in one part of the song. */
USTRUCT(BlueprintType)
struct FSpotlightSpawnRule
{
	GENERATED_BODY()

	/** 한 번 스폰한 뒤 다음 스폰까지 기다리는 시간 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "스폰 간격"))
	FGimmickInterval Interval;

	/** 한 번에 생기는 스포트라이트의 최소 개수 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "최소 스폰 개수", ClampMin = "0"))
	int32 MinSpawnCount = 1;

	/** 한 번에 생기는 스포트라이트의 최대 개수 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "최대 스폰 개수", ClampMin = "0"))
	int32 MaxSpawnCount = 2;
};

/**
 * Settings of ASpotlightManager.
 *
 * @see ASpotlightManager
 */
UCLASS(meta = (DisplayName = "스포트라이트"))
class TROMBONERUMBLE_API USpotlightGimmickConfig : public UGimmickConfig
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
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	/** @return The rule for the current part of the song. */
	const FSpotlightSpawnRule& GetRule(const bool bIsFeverTime) const { return bIsFeverTime ? Fever : Normal; }

	/** 곡의 피버 큐 전에 쓰는 스폰 규칙 */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "일반"))
	FSpotlightSpawnRule Normal;

	/** 곡의 피버 큐부터 쓰는 스폰 규칙 */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "피버"))
	FSpotlightSpawnRule Fever;

	/** 스포트라이트가 생긴 뒤 켜지기까지의 시간. 플레이어는 이 동안 안으로 들어간다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "예고 시간", ClampMin = "0.1", Units = "s"))
	float WarningDuration = 1.5f;

	/** 스포트라이트가 켜져 있는 시간. 이 동안 안에서 연주에 성공하면 점수를 얻는다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "발동 시간", ClampMin = "0.1", Units = "s"))
	float ActiveDuration = 3.f;

	/** 켜진 시간이 끝난 뒤 스포트라이트가 서서히 사라지는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "연출", meta = (DisplayName = "사라지는 시간", ClampMin = "0.1", Units = "s"))
	float FadeDuration = 2.f;

	/** 스포트라이트 안에서 연주에 성공하면 얻는 점수 */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "성공 보너스 점수", ClampMin = "0"))
	int32 BonusScore = 70;
};
