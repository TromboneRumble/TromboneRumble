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
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	/** @return Seconds one flood takes from the start of the warning to the end of the drain. */
	float GetFloodDuration() const { return WarningDuration + RisingDuration + SustainDuration + DrainingDuration; }

	/** Seconds from the start of the gimmick to the first warning. */
	UPROPERTY(EditAnywhere, Category = "Schedule", meta = (DisplayName = "첫 전조 시작 시간", ClampMin = "0.0", Units = "s"))
	float FirstWarningDelay = 30.f;

	/**
	 * Seconds from the start of one warning to the start of the next warning.
	 * The flood times below do not change it.
	 */
	UPROPERTY(EditAnywhere, Category = "Schedule", meta = (DisplayName = "재발생 주기 (전조 시작 기준)", ClampMin = "1.0", Units = "s"))
	float Period = 30.f;

	/** Seconds of warning before the beer starts to rise. */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "전조 시간", ClampMin = "0.0", Units = "s"))
	float WarningDuration = 5.f;

	/** Seconds the beer takes to reach its top. */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "수위 상승 시간", ClampMin = "0.05", Units = "s"))
	float RisingDuration = 3.f;

	/** Seconds the beer stays at its top. */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "침수 유지 시간", ClampMin = "0.0", Units = "s"))
	float SustainDuration = 5.f;

	/** Seconds the beer takes to drain. */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "배수 시간", ClampMin = "0.05", Units = "s"))
	float DrainingDuration = 3.f;

	/** How far the surface rises above the gimmick actor, in cm. */
	UPROPERTY(EditAnywhere, Category = "BeerFlood", meta = (DisplayName = "차오르는 높이", ClampMin = "0.0", Units = "cm"))
	float FloodHeight = 100.f;
};
