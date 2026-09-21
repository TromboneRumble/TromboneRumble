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

	/** Wait between two spawn rounds. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "스폰 간격"))
	FGimmickInterval Interval;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "최소 스폰 개수", ClampMin = "0"))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "최대 스폰 개수", ClampMin = "0"))
	int32 MaxCount = 2;
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
		Normal.MinCount = 1;
		Normal.MaxCount = 2;

		Fever.Interval.Min = 5.f;
		Fever.Interval.Max = 8.f;
		Fever.MinCount = 2;
		Fever.MaxCount = 4;
	}

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface
#endif

	/** @return The rule for the current part of the song. */
	const FSpotlightSpawnRule& GetRule(const bool bIsFeverTime) const { return bIsFeverTime ? Fever : Normal; }

	/** Used before the fever cue of the song. */
	UPROPERTY(EditAnywhere, Category = "Spotlight", meta = (DisplayName = "일반"))
	FSpotlightSpawnRule Normal;

	/** Used after the fever cue of the song. */
	UPROPERTY(EditAnywhere, Category = "Spotlight", meta = (DisplayName = "피버"))
	FSpotlightSpawnRule Fever;

	/** Seconds from the spawn of a spotlight until it turns on. Players use this time to get inside. */
	UPROPERTY(EditAnywhere, Category = "Spotlight", meta = (DisplayName = "경고 상태 지속 시간", ClampMin = "0.0", Units = "s"))
	float WarningDuration = 1.5f;

	/** Seconds a spotlight stays on. A play inside it scores during this time. */
	UPROPERTY(EditAnywhere, Category = "Spotlight", meta = (DisplayName = "활성 상태 지속 시간", ClampMin = "0.0", Units = "s"))
	float ActiveDuration = 3.f;

	/** Seconds a spotlight takes to fade out after its active time ran out. */
	UPROPERTY(EditAnywhere, Category = "Spotlight", meta = (DisplayName = "소멸 단계 지속 시간", ClampMin = "0.0", Units = "s"))
	float FadingDuration = 2.f;

	/** Score for a successful play inside a spotlight. */
	UPROPERTY(EditAnywhere, Category = "Spotlight", meta = (DisplayName = "성공 보너스 점수", ClampMin = "0"))
	int32 BonusScore = 70;
};
