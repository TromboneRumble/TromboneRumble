// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "PresentGimmickConfig.generated.h"

class APresent;

/**
 * Settings of APresentSpawner. The docking port supply of the space station uses the same config with another present class.
 *
 * @see APresentSpawner
 */
UCLASS(meta = (DisplayName = "선물"))
class TROMBONERUMBLE_API UPresentGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UPresentGimmickConfig() { GimmickType = EGimmickType::Present; }

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	/** Present Blueprint dropped in this level. */
	UPROPERTY(EditAnywhere, Category = "Present", meta = (DisplayName = "선물 클래스"))
	TSubclassOf<APresent> PresentClass;

	/** Seconds between two drops. 0 stops the drops. */
	UPROPERTY(EditAnywhere, Category = "Present", meta = (DisplayName = "선물 생성 간격", ClampMin = "0.0", Units = "s"))
	float SpawnInterval = 10.f;

	/** Score for picking up one present. */
	UPROPERTY(EditAnywhere, Category = "Present", meta = (DisplayName = "선물 획득 점수", ClampMin = "0"))
	int32 BonusScore = 300;
};
