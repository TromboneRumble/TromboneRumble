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

	/** Is the first warning at a fixed time. Off picks it between IntervalMin and IntervalMax like every later one. */
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bFixedFirstDelay = false;

	/** Seconds from the start of the gimmick to the first warning. */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "첫 발동 시간", ClampMin = "0.0", Units = "s", EditCondition = "bFixedFirstDelay"))
	float FirstDelay = 0.f;

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

	/** @return FirstDelay when it is fixed, or a random interval. */
	float PickFirstDelay() const { return bFixedFirstDelay ? FirstDelay : PickInterval(); }
};

/**
 * UEventGimmickConfig is the base for gimmicks that wait a random time after one event ends and then warn again.
 * Gravity and black hole use it. Beer flood and drunkard wait a fixed cooldown and have their own fields.
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API UEventGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	/**
	 * Add warn, run, wait and repeat spans that follow Schedule, for BuildTimeline of a subclass.
	 *
	 * @param EventDuration Seconds the event runs after its warning.
	 * @param EventLabel Name of the event span in the tooltip.
	 */
	void BuildScheduleTimeline(FGimmickTimelineBuilder& Builder, float EventDuration, const FText& EventLabel) const;
#endif

	UPROPERTY(EditAnywhere, Category = "Schedule", meta = (DisplayName = "일정"))
	FGimmickSchedule Schedule;
};
