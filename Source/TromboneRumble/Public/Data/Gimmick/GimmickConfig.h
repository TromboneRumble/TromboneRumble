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

#if WITH_EDITOR
	/** Add an error to Context for each value that would stop the gimmick from working, for example an empty class list. */
	virtual void ValidateConfig(FDataValidationContext& Context) const {}

	/**
	 * Add the spans this gimmick is expected to show during one round, for the timeline of the gimmick settings panel.
	 * Repeat the timer rule of the gimmick actor with the values of this config.
	 */
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const {}
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

	UPROPERTY(EditAnywhere, meta = (DisplayName = "최소", ClampMin = "0.0", Units = "s"))
	float Min = 5.f;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "최대", ClampMin = "0.0", Units = "s"))
	float Max = 10.f;

	/** @return A random wait between Min and Max. */
	float Pick() const { return FMath::RandRange(Min, FMath::Max(Min, Max)); }
};

/** Timing shared by every gimmick that starts, warns, runs and waits again. */
USTRUCT(BlueprintType)
struct FGimmickSchedule
{
	GENERATED_BODY()

	/** Seconds from the start of the round to the first warning. Below 0 uses a normal interval instead. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "첫 발동 지연 (-1 = 발동 간격 사용)", ClampMin = "-1.0", Units = "s"))
	float FirstDelay = -1.f;

	/** Shortest wait in seconds from the end of one event to the next warning. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "발동 간격 최소", ClampMin = "0.0", Units = "s"))
	float IntervalMin = 30.f;

	/** Longest wait in seconds from the end of one event to the next warning. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "발동 간격 최대", ClampMin = "0.0", Units = "s"))
	float IntervalMax = 45.f;

	/** Seconds of warning before the gimmick starts. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "예고 시간", ClampMin = "0.0", Units = "s"))
	float WarningDuration = 4.f;

	/** @return A random wait between IntervalMin and IntervalMax. */
	float PickInterval() const { return FMath::RandRange(IntervalMin, FMath::Max(IntervalMin, IntervalMax)); }

	/** @return FirstDelay, or a normal interval when FirstDelay is below 0. */
	float PickFirstDelay() const { return FirstDelay >= 0.f ? FirstDelay : PickInterval(); }
};

/**
 * UEventGimmickConfig is the base for gimmicks that wait a random time after one event ends and then warn again.
 * Gravity uses it. Beer flood repeats on a fixed period and drunkard has its own timing, so they do not.
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API UEventGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Schedule", meta = (DisplayName = "일정"))
	FGimmickSchedule Schedule;
};
