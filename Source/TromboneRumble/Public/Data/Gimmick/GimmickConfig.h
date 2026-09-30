// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Utilities/Defines.h"
#include "GimmickConfig.generated.h"

class AGimmickBase;
class FDataValidationContext;
class FGimmickTimelineBuilder;

/**
 * UGimmickConfig holds the settings a designer tunes for one gimmick.
 * Each gimmick has one subclass. UStageGimmickData keeps one instance per gimmick used in a level.
 * The class defaults are the project wide defaults, and a level changes only the values it needs.
 *
 * Keep in mind that only settings a designer tunes belong here.
 * Meshes, sounds, effects and level references stay on the gimmick actor.
 *
 * @see UStageGimmickData
 * @see AGimmickBase
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class TROMBONERUMBLE_API UGimmickConfig : public UObject
{
	GENERATED_BODY()

public:

	/** @return The gimmick this config belongs to. A gimmick actor with the same type reads it. */
	EGimmickType GetGimmickType() const { return GimmickType; }

	/** @return Gimmick Blueprint the manager spawns when the level has no gimmick of this type. Can be null. */
	TSubclassOf<AGimmickBase> GetGimmickClass() const { return GimmickClass; }

	/**
	 * @return Whether the gimmick starts only when a sequence of the stage data gives it a turn.
	 *         Such a gimmick has no timer of its own and must be in a sequence, or it never starts.
	 */
	virtual bool RunsOnlyInSequence() const { return false; }

#if WITH_EDITOR
	/** Add an error to Context for each value that would stop the gimmick from working, for example an empty class list. */
	virtual void ValidateConfig(FDataValidationContext& Context) const {}

	/**
	 * Add the spans this gimmick is expected to show during one round, for the timeline of the gimmick settings panel.
	 * Repeat the timer rule of the gimmick actor with the values of this config.
	 */
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const {}

	/**
	 * Add the spans of one event that starts at Start. The panel calls it for gimmicks in a sequence.
	 *
	 * @return Time the event ends.
	 */
	virtual float BuildEventTimeline(FGimmickTimelineBuilder& Builder, float Start) const { return Start; }
#endif

protected:

	/**
	 * When the level has no gimmick of this type, the manager spawns this Blueprint at its own location.
	 * Shown only on configs that set bSpawnableByManager.
	 */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "매니저가 스폰할 기믹 BP", EditCondition = "bSpawnableByManager", EditConditionHides))
	TSubclassOf<AGimmickBase> GimmickClass;

	/**
	 * Can the manager spawn this gimmick. Set it in the constructor of a config whose gimmick needs no place in the level, such as gravity.
	 * A gimmick that holds spawn points or other level references must be placed by hand, so it leaves this false and GimmickClass stays hidden.
	 */
	UPROPERTY(Transient)
	bool bSpawnableByManager = false;


	/** Set in the constructor of each subclass. */
	EGimmickType GimmickType = EGimmickType::None;
};

/** A random wait in seconds between two values. Used by spawners. */
USTRUCT(BlueprintType)
struct FGimmickInterval
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (DisplayName = "최소", ClampMin = "0.1", Units = "s"))
	float Min = 5.f;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "최대", ClampMin = "0.1", Units = "s"))
	float Max = 10.f;

	/** @return A random wait between Min and Max. */
	float Pick() const { return FMath::RandRange(Min, FMath::Max(Min, Max)); }
};

/**
 * UEventGimmickConfig is the base for gimmicks that warn and then run one event when a sequence gives them a turn.
 * Gravity, black hole and UFO use it.
 *
 * Keep in mind that these gimmicks have no timer of their own.
 * A sequence of the stage data must list them, or they never start.
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API UEventGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	//~ Begin UGimmickConfig Interface
	virtual bool RunsOnlyInSequence() const override { return true; }
	//~ End UGimmickConfig Interface

	/** 차례가 오면 예고를 시작하고 이 시간 뒤 발동한다. 0이면 예고 없이 바로 발동한다 (초) */
	UPROPERTY(EditAnywhere, Category = "Schedule", meta = (DisplayName = "예고 시간", ClampMin = "0.0", Units = "s"))
	float WarningDuration = 4.f;
};
