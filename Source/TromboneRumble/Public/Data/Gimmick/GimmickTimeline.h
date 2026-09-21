// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"

/** What a gimmick does during one span of the timeline. The timeline widget picks the color from it. */
enum class EGimmickTimelinePhase : uint8
{
	/** The gimmick warns the players and does nothing to them yet. */
	Warning,

	/** The gimmick affects the players. */
	Active,

	/** The gimmick still shows but goes away, such as the beer draining. */
	Ending,

	/** One object spawns at this moment. The span has no length. */
	Spawn,
};

/** One block on the timeline row of a gimmick. */
struct FGimmickTimelineSpan
{
	/** Seconds from the moment the gimmicks are activated. */
	float Start = 0.f;

	/** Length in seconds. 0 for a spawn. */
	float Duration = 0.f;

	EGimmickTimelinePhase Phase = EGimmickTimelinePhase::Active;

	/** Name of the phase shown in the tooltip, such as "수위 상승". */
	FText Label;
};

/**
 * FGimmickTimelineBuilder collects the spans one gimmick config predicts for a round.
 * The gimmick settings panel creates one per config and passes it to UGimmickConfig::BuildTimeline.
 *
 * Keep in mind that a config must repeat the timer rule of its gimmick actor here.
 * When the rule of the actor changes, change BuildTimeline of its config too.
 *
 * @see UGimmickConfig
 */
class FGimmickTimelineBuilder
{
public:

	/** A loop that advances by less than this would never end when a designer types 0 as an interval. */
	static constexpr float MinStep = 0.5f;

	FGimmickTimelineBuilder(const float InRoundLength, const float InFeverStart, const int32 Seed)
		: RoundLength(InRoundLength)
		, FeverStart(InFeverStart)
		, Random(Seed)
	{
	}

	/** @return Whether Time is before the end of the round. Use it as the loop condition. */
	bool IsInRound(const float Time) const { return Time < RoundLength; }

	/** Whether Time is in fever time. Calling it also makes the row show where fever time starts. */
	bool IsFever(const float Time)
	{
		bUsesFever = true;
		return Time >= FeverStart;
	}

	/** @return A random wait between Min and Max. The same seed gives the same timeline. */
	float Pick(const float Min, const float Max) { return Random.FRandRange(Min, FMath::Max(Min, Max)); }

	/**
	 * Add one span.
	 *
	 * @return Time the span ends, so the next span can start there.
	 */
	float AddSpan(const float Start, const float Duration, const EGimmickTimelinePhase Phase, const FText& Label)
	{
		if (IsInRound(Start))
		{
			Spans.Add({ Start, FMath::Max(0.f, Duration), Phase, Label });
		}
		return Start + FMath::Max(0.f, Duration);
	}

	/** Add a note shown in the tooltip of the row label, such as a limit of the prediction. */
	void SetNote(const FText& InNote) { Note = InNote; }

	const TArray<FGimmickTimelineSpan>& GetSpans() const { return Spans; }

	const FText& GetNote() const { return Note; }

	bool UsesFever() const { return bUsesFever; }

private:

	float RoundLength;

	float FeverStart;

	FRandomStream Random;

	TArray<FGimmickTimelineSpan> Spans;

	FText Note;

	/** Has the config asked about fever time. */
	bool bUsesFever = false;
};
