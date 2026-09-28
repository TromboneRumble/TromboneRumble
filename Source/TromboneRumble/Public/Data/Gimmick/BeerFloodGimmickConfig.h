// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "BeerFloodGimmickConfig.generated.h"

/**
 * Settings of ABeerFloodGimmick.
 *
 * @see ABeerFloodGimmick
 */
UCLASS(meta = (DisplayName = "술통 침수"))
class TROMBONERUMBLE_API UBeerFloodGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UBeerFloodGimmickConfig()
	{
		GimmickType = EGimmickType::BeerFlood;
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual float BuildEventTimeline(FGimmickTimelineBuilder& Builder, float Start) const override;
	//~ End UGimmickConfig Interface
#endif

	//~ Begin UGimmickConfig Interface
	virtual bool RunsOnlyInSequence() const override { return true; }
	//~ End UGimmickConfig Interface

	/** @return Seconds one flood takes from the start of the warning to the end of the drain. */
	float GetFloodDuration() const { return WarningDuration + RisingDuration + SustainDuration + DrainingDuration; }

	/** 술통이 부풀어 오르며 침수를 예고하는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "전조 시간", ClampMin = "0.0", Units = "s"))
	float WarningDuration = 5.f;

	/** 맥주가 최고 수위까지 차오르는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "수위 상승 시간", ClampMin = "0.05", Units = "s"))
	float RisingDuration = 3.f;

	/** 최고 수위에서 침수가 유지되는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "침수 유지 시간", ClampMin = "0.0", Units = "s"))
	float SustainDuration = 5.f;

	/** 맥주가 빠져 원래 수위로 돌아가는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "배수 시간", ClampMin = "0.05", Units = "s"))
	float DrainingDuration = 3.f;

	/** 기믹 액터 위치에서 수면이 올라오는 높이 (cm) */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "차오르는 높이", ClampMin = "0.0", Units = "cm"))
	float FloodHeight = 100.f;
};
