// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GimmickSpawnerBase.generated.h"

/**
 * AGimmickSpawnerBase runs the spawn timer of every gimmick that spawns objects again and again.
 * After the first spawn delay it spawns a batch, then waits a random interval and spawns again, until the gimmick is turned off.
 * Each spawn reads the rule of that moment from USpawnerGimmickConfig, so fever time and an edit made during play take effect from the next spawn.
 *
 * Keep in mind that a subclass only spawns. It returns its config from GetSpawnerConfig and makes the objects in SpawnBatch.
 * The timer runs on the server only.
 *
 * @see USpawnerGimmickConfig
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API AGimmickSpawnerBase : public AGimmickBase
{
	GENERATED_BODY()

public:

	//~ Begin AGimmickBase Interface
	virtual void Activate() override;
	virtual void ForceTrigger() override;
	//~ End AGimmickBase Interface

protected:

	/**
	 * @return Config of this spawner. Return GetConfig with the config class of the gimmick,
	 *         so a level without an entry falls back to the defaults of that class.
	 */
	virtual const USpawnerGimmickConfig& GetSpawnerConfig() const PURE_VIRTUAL(AGimmickSpawnerBase::GetSpawnerConfig, return *GetDefault<USpawnerGimmickConfig>(););

	/**
	 * Spawn this many objects now. Server only.
	 * Spawn fewer when there is not enough room, and none when an earlier batch still keeps the spawner busy.
	 */
	virtual void SpawnBatch(int32 Count) PURE_VIRTUAL(AGimmickSpawnerBase::SpawnBatch, );

	/** @return Whether fever time has started in this round. Server only. */
	bool IsFeverTime() const;

private:

	/** Spawn a batch with the rule of this moment, then wait for the next one. */
	void HandleSpawnTimer();

	FTimerHandle SpawnTimerHandle;
};
