// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/EventGimmickBase.h"

void AEventGimmickBase::Deactivate()
{
	// An event stopped half way still ends its turn, so a sequence waiting for it does not stall
	// The flag goes down first, so a FinishEvent from the cleanup does nothing
	if (HasAuthority() && bEventRunning)
	{
		bEventRunning = false;
		OnEventStopped();
		NotifyEventFinished();
	}

	// Super clears the timers
	Super::Deactivate();
}

void AEventGimmickBase::ForceTrigger()
{
	if (!HasAuthority() || bEventRunning) return;

	if (!PrepareEvent())
	{
		NotifyEventFinished();
		return;
	}

	bEventRunning = true;

	// No warning time means no warning, and the first phase starts in this frame
	const UEventGimmickConfig& Config = GetEventConfig();
	if (!Config.HasWarning() || Config.WarningDuration <= 0.f)
	{
		StartPhase(0);
		return;
	}

	OnWarningStarted();
	StartPhaseTimer(GetWorldTimerManager(), PhaseTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::StartPhase, 0), Config.WarningDuration);
}

void AEventGimmickBase::FinishEvent()
{
	if (!HasAuthority() || !bEventRunning) return;

	bEventRunning = false;
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);

	OnEventEnded();
	NotifyEventFinished();
}

void AEventGimmickBase::StartPhase(const int32 PhaseIndex)
{
	if (!HasAuthority() || !bEventRunning) return;

	if (!RunsPhaseTimers())
	{
		OnEventPhaseStarted(PhaseIndex);
		return;
	}

	// Read again for each phase, so a value edited during play counts from the next phase
	TArray<FGimmickPhase> Phases;
	GetEventConfig().GetEventPhases(GetWorld(), Phases);
	if (!Phases.IsValidIndex(PhaseIndex))
	{
		FinishEvent();
		return;
	}

	OnEventPhaseStarted(PhaseIndex);
	StartPhaseTimer(GetWorldTimerManager(), PhaseTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::StartPhase, PhaseIndex + 1), Phases[PhaseIndex].Duration);
}
