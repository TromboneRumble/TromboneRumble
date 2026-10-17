// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/GimmickSpawnerBase.h"
#include "Actors/Gimmick/GimmickManager.h"

void AGimmickSpawnerBase::Activate()
{
	const bool bWasActive = IsActive();
	Super::Activate();

	// A second Activate must not restart the wait of the first
	if (bWasActive || !HasAuthority()) return;

	StartPhaseTimer(GetWorldTimerManager(), SpawnTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::HandleSpawnTimer), GetSpawnerConfig().FirstSpawnDelay);
}

void AGimmickSpawnerBase::ForceTrigger()
{
	if (!HasAuthority()) return;

	// One more batch now. The timer keeps its own pace
	SpawnBatch(GetSpawnerConfig().GetSpawnRule(IsFeverTime()).PickSpawnCount());
}

bool AGimmickSpawnerBase::IsFeverTime() const
{
	const AGimmickManager* Manager = AGimmickManager::Find(GetWorld());
	return Manager && Manager->IsFeverTime();
}

void AGimmickSpawnerBase::HandleSpawnTimer()
{
	const FGimmickSpawnRule& Rule = GetSpawnerConfig().GetSpawnRule(IsFeverTime());
	SpawnBatch(Rule.PickSpawnCount());

	StartPhaseTimer(GetWorldTimerManager(), SpawnTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::HandleSpawnTimer), Rule.Interval.Pick());
}
