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
UCLASS(meta = (DisplayName = "중력"))
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
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	/** Seconds gravity stays changed. */
	UPROPERTY(EditAnywhere, Category = "Gravity", meta = (DisplayName = "지속 시간", ClampMin = "0.1", Units = "s"))
	float ActiveDuration = 10.f;

	/** Gravity is multiplied by this while active. Below 1 floats, above 1 pulls down. */
	UPROPERTY(EditAnywhere, Category = "Gravity", meta = (DisplayName = "중력 배율", ClampMin = "0.05", ClampMax = "20.0"))
	float GravityMultiplier = 0.3f;
};
