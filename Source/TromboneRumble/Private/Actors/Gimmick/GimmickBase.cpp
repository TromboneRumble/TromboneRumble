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

const UGimmickConfig* AGimmickBase::FindConfig() const
{
	if (bConfigSearched) return CachedConfig.Get();

	const UWorld* World = GetWorld();
	if (!World) return nullptr;

	// The editor world is never cached, because the manager there can get another asset at any time
	bConfigSearched = World->IsGameWorld();

	// We ask the manager instead of waiting for it to set the config
	// A gimmick can need its config before the manager reaches BeginPlay, for example in an OnRep on a client
	CachedConfig = AGimmickManager::FindConfigInWorld(World, GimmickType);

	return CachedConfig.Get();
}
