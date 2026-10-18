// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "EventGimmickBase.generated.h"

/**
 * AEventGimmickBase runs one event when a sequence of the stage data gives the gimmick its turn.
 * An event is a warning, then the phases, then the end.
 * It owns the rules every event shares: start only from idle, and skip a warning of 0 seconds.
 * It also tells the manager when the event is over, even when the event is stopped half way.
 *
 * The phases come from UEventGimmickConfig::GetEventPhases, the same list the timeline of the gimmick settings panel draws.
 * A gimmick that runs its phases on timers only says what to do when each phase starts, so the game and the timeline cannot drift apart.
 * A gimmick that ends on its own, such as the UFO or the drunkard, gets phase 0 without a timer and calls FinishEvent itself.
 *
 * Keep in mind that a subclass keeps its own state enum for its visuals and Blueprints, and sets it from the hooks below.
 *
 * @see UEventGimmickConfig
 * @see AGimmickSpawnerBase
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API AEventGimmickBase : public AGimmickBase
{
	GENERATED_BODY()

public:

	//~ Begin AGimmickBase Interface
	virtual void Deactivate() override;
	virtual void ForceTrigger() override final;
	//~ End AGimmickBase Interface

protected:

	/**
	 * @return Config of this event. Return GetConfig with the config class of the gimmick,
	 *         so a level without an entry falls back to the defaults of that class.
	 */
	virtual const UEventGimmickConfig& GetEventConfig() const PURE_VIRTUAL(AEventGimmickBase::GetEventConfig, return *GetDefault<UEventGimmickConfig>(););

	/**
	 * Get ready before the warning, such as picking where the event happens. Server only.
	 *
	 * @return False when the event cannot run. The turn then ends at once, so a sequence never waits for it.
	 */
	virtual bool PrepareEvent() { return true; }

	/** The warning starts. Not called when the warning is 0 seconds or the event has none. Server only. */
	virtual void OnWarningStarted() {}

	/**
	 * A phase of GetEventPhases starts. Server only.
	 *
	 * @param PhaseIndex Index in the list of GetEventPhases. Always 0 for an event that does not run phase timers.
	 */
	virtual void OnEventPhaseStarted(int32 PhaseIndex) {}

	/** The last phase is over, or the event finished itself. The manager is told right after. Server only. */
	virtual void OnEventEnded() {}

	/** The gimmick was turned off while the event ran. The default cleans up the same way as OnEventEnded. Server only. */
	virtual void OnEventStopped() { OnEventEnded(); }

	/** @return Whether the phases end on timers. False for an event that calls FinishEvent itself. */
	virtual bool RunsPhaseTimers() const { return true; }

	/** End the running event now. Does nothing when no event runs. Server only. */
	void FinishEvent();

	/** Timer of the warning and of the current phase. */
	FTimerHandle PhaseTimerHandle;

private:

	/** Start a phase, or finish the event after the last one. Server only. */
	void StartPhase(int32 PhaseIndex);

	/** Is an event running, from its warning to its end. Server only. */
	bool bEventRunning = false;
};
