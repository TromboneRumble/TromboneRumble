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
class TROMBONERUMBLE_API UBeerFloodGimmickConfig : public UEventGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UBeerFloodGimmickConfig()
	{
		GimmickType = EGimmickType::BeerFlood;
		WarningDuration = 5.f;
	}

	//~ Begin UEventGimmickConfig Interface
	virtual void GetEventPhases(const UWorld* World, TArray<FGimmickEventPhase>& OutPhases) const override;
	//~ End UEventGimmickConfig Interface

	/** 맥주가 최고 수위까지 차오르는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "수위 상승 시간", ClampMin = "0.0", Units = "s"))
	float RisingDuration = 3.f;

	/** 최고 수위에서 침수가 유지되는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "침수 유지 시간", ClampMin = "0.0", Units = "s"))
	float SustainDuration = 5.f;

	/** 맥주가 빠져 원래 수위로 돌아가는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "배수 시간", ClampMin = "0.0", Units = "s"))
	float DrainingDuration = 3.f;

	/** 기믹 액터 위치에서 수면이 올라오는 높이 (cm) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "차오르는 높이", ClampMin = "0.0", Units = "cm"))
	float FloodHeight = 100.f;
};
