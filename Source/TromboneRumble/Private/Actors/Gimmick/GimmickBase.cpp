// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/GimmickBase.h"
#include "Actors/Gimmick/GimmickManager.h"

AGimmickBase::AGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGimmickBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Deactivate();
	
	Super::EndPlay(EndPlayReason);
}

void AGimmickBase::Activate()
{
	if (bIsActive) return;
	
	bIsActive = true;
}

void AGimmickBase::Deactivate()
{
	if (!bIsActive) return;
	
	bIsActive = false;
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void AGimmickBase::StartPhaseTimer(FTimerManager& TimerManager, FTimerHandle& Handle, const FTimerDelegate& Callback, const float Duration)
{
	TimerManager.ClearTimer(Handle);

	if (Duration > 0.f)
	{
		TimerManager.SetTimer(Handle, Callback, Duration, false);
		return;
	}

	// Next tick and not at once, so the phase still starts and replicates before it ends
	Handle = TimerManager.SetTimerForNextTick(Callback);
}

void AGimmickBase::NotifyEventFinished()
{
	if (AGimmickManager* Manager = AGimmickManager::Find(GetWorld()))
	{
		Manager->HandleEventFinished(*this);
	}
}

const UGimmickConfig* FGimmickConfigCache::Get(const UWorld* World, const EGimmickType GimmickType) const
{
	if (bSearched) return Config.Get();
	if (!World) return nullptr;

	// The editor world is never cached, because the manager there can get another asset at any time
	bSearched = World->IsGameWorld();
	Config = AGimmickManager::FindConfigInWorld(World, GimmickType);

	return Config.Get();
}
