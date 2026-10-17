// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/GimmickTimeline.h"
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
 * A phase time of 0 skips the phase, so its minimum is 0. The actor starts the phase with AGimmickBase::StartPhaseTimer to keep that true.
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

	/** 레벨에 이 기믹이 없을 때 매니저가 자기 위치에 스폰하는 기믹 블루프린트 */
	UPROPERTY(EditAnywhere, Category = "스폰", meta = (DisplayName = "매니저가 스폰할 기믹 BP", EditCondition = "bSpawnableByManager", EditConditionHides, HideEditConditionToggle))
	TSubclassOf<AGimmickBase> GimmickClass;

	/**
	 * Can the manager spawn this gimmick. Set it in the constructor of a config whose gimmick needs no place in the level, such as gravity.
	 * GimmickClass hides the checkbox the details view would add for it, because a designer must not turn it off.
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

/** How often a spawner spawns and how many objects it makes at once, for one part of the song. */
USTRUCT(BlueprintType)
struct FGimmickSpawnRule
{
	GENERATED_BODY()

	/** 한 번 스폰한 뒤 다음 스폰까지 기다리는 시간 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "스폰 간격"))
	FGimmickInterval Interval;

	/** 한 번에 만드는 최소 개수. 빈 자리가 모자라면 있는 만큼만 만든다 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "최소 스폰 개수", ClampMin = "1"))
	int32 MinSpawnCount = 1;

	/** 한 번에 만드는 최대 개수 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "최대 스폰 개수", ClampMin = "1"))
	int32 MaxSpawnCount = 1;

	/** @return A random count between MinSpawnCount and MaxSpawnCount. */
	int32 PickSpawnCount() const { return FMath::RandRange(MinSpawnCount, FMath::Max(MinSpawnCount, MaxSpawnCount)); }
};

/**
 * USpawnerGimmickConfig is the base for gimmicks that spawn objects again and again on their own timer.
 * Spotlight, garbage, the bonus drops and the water drops use it, and AGimmickSpawnerBase runs the timer for all of them.
 *
 * Keep in mind that a subclass sets its default rule in its constructor, so a level without an entry still spawns the way the gimmick was designed.
 * A gimmick is turned off by taking its entry out of the stage data, not by a special value.
 *
 * @see AGimmickSpawnerBase
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API USpawnerGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	/** @return The rule for the current part of the song. The fever rule counts only while bUseFeverRule is on. */
	const FGimmickSpawnRule& GetSpawnRule(const bool bIsFeverTime) const { return bIsFeverTime && bUseFeverRule ? Fever : Normal; }

#if WITH_EDITOR
	//~ Begin UGimmickConfig Interface
	virtual void ValidateConfig(FDataValidationContext& Context) const override;
	virtual void BuildTimeline(FGimmickTimelineBuilder& Builder) const override;
	//~ End UGimmickConfig Interface

	/**
	 * Add the spans of one spawn at SpawnTime, for the timeline. The default marks the spawn only.
	 *
	 * @return Time the spawner stays busy until. A spawn that comes earlier is skipped, the same as the spawner does in the game.
	 */
	virtual float BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, float SpawnTime) const;
#endif

	/** 기믹이 켜진 뒤 첫 스폰까지의 시간. 0이면 켜지자마자 스폰한다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이|스폰", meta = (DisplayName = "첫 스폰 지연", ClampMin = "0.0", Units = "s"))
	float FirstSpawnDelay = 0.f;

	/** 평소에 쓰는 스폰 규칙. 피버 타임 시 강화를 끄면 피버 타임에도 이 규칙을 쓴다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|스폰", meta = (DisplayName = "일반"))
	FGimmickSpawnRule Normal;

	/** 켜면 피버 타임부터 피버 규칙의 간격과 개수로 스폰한다. 기다리던 간격은 그대로 끝나고 그다음 스폰부터 바뀐다. 끄면 곡 내내 일반 규칙을 쓴다 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|스폰", meta = (DisplayName = "피버 타임 시 강화"))
	bool bUseFeverRule = false;

	/** 피버 타임 시 강화를 켰을 때 피버 타임부터 쓰는 스폰 규칙 */
	UPROPERTY(EditAnywhere, Category = "게임플레이|스폰", meta = (DisplayName = "피버", EditCondition = "bUseFeverRule"))
	FGimmickSpawnRule Fever;
};

/** One phase of an event after its warning, such as the rise of the beer. */
struct FGimmickEventPhase
{
	/** Kind of the phase. It also decides the color on the timeline. */
	EGimmickTimelinePhase Phase = EGimmickTimelinePhase::Active;

	/** Name shown on the timeline, such as "수위 상승". */
	FText Label;

	/** Length in seconds. */
	float Duration = 0.f;
};

/**
 * UEventGimmickConfig is the base for gimmicks that run one event when a sequence gives them a turn.
 * Gravity, black hole, UFO, beer flood and the drunkard use it.
 * An event warns first, then goes through the phases GetEventPhases lists, then tells the manager it is over.
 *
 * Keep in mind that these gimmicks have no timer of their own.
 * A sequence of the stage data must list them, or they never start.
 * GetEventPhases must follow the timer rule of the gimmick actor, because the timeline and GetEventDuration read it.
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API UEventGimmickConfig : public UGimmickConfig
{
	GENERATED_BODY()

public:

	//~ Begin UGimmickConfig Interface
	virtual bool RunsOnlyInSequence() const override { return true; }
#if WITH_EDITOR
	virtual float BuildEventTimeline(FGimmickTimelineBuilder& Builder, float Start) const override;
#endif
	//~ End UGimmickConfig Interface

	/**
	 * Add the phases of one event after its warning, in order.
	 *
	 * @param World Level the event runs in. Null when there is none, and a phase that depends on the level uses a guess then.
	 * @param OutPhases The phases are added here.
	 */
	virtual void GetEventPhases(const UWorld* World, TArray<FGimmickEventPhase>& OutPhases) const {}

	/**
	 * @return Seconds one event lasts from the start of its warning, when nothing ends it early.
	 *         A phase of 0 seconds adds nothing, the same as the gimmick skips it.
	 */
	float GetEventDuration(const UWorld* World = nullptr) const
	{
		TArray<FGimmickEventPhase> Phases;
		GetEventPhases(World, Phases);

		float Duration = bHasWarning ? FMath::Max(WarningDuration, 0.f) : 0.f;
		for (const FGimmickEventPhase& Phase : Phases)
		{
			Duration += FMath::Max(Phase.Duration, 0.f);
		}
		return Duration;
	}

	/** @return Whether the event warns before it starts. */
	bool HasWarning() const { return bHasWarning; }

	/** 기믹 시퀀스에서 차례가 오거나 강제 발동하면 예고를 시작하고, 이 시간 뒤 발동한다. 0이면 예고 없이 바로 발동한다 (초) */
	UPROPERTY(EditAnywhere, Category = "게임플레이", meta = (DisplayName = "예고 시간", ClampMin = "0.0", Units = "s", EditCondition = "bHasWarning", EditConditionHides, HideEditConditionToggle))
	float WarningDuration = 4.f;

protected:

	/**
	 * Does the event warn before it starts. Set it to false in the constructor of an event without a warning, such as the drunkard.
	 * WarningDuration is then hidden, and it hides the checkbox the details view would add for this, because a designer must not turn it on.
	 */
	UPROPERTY(Transient)
	bool bHasWarning = true;
};
