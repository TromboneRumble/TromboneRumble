// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "BonusSpawner.generated.h"

class ABonusDrop;
class ATargetPoint;

/**
 * BonusSpawner drops ABonusDrop actors on some of its spawn points at a fixed interval.
 * The present of the snow field and the docking port supply of the space station both use it, and the Blueprint picks the gimmick type.
 * Every number comes from UBonusGimmickConfig.
 *
 * Each round picks free points, shows a warning marker on the floor below them, then drops one actor on each.
 * A point is free while no drop from an earlier round sits on it.
 *
 * @see ABonusDrop
 * @see UBonusGimmickConfig
 */
UCLASS()
class TROMBONERUMBLE_API ABonusSpawner : public AGimmickBase
{
	GENERATED_BODY()

public:

	ABonusSpawner();

	//~ Begin AGimmickBase Interface
	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void ForceTrigger() override;
	//~ End AGimmickBase Interface

protected:

	/** Places a drop falls from. Put them in the air above the floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

private:

	/** Pick the points of this round, then warn or drop at once. Server only. */
	void StartDrop();

	/** Drop one actor on each point this round picked. Server only. */
	void DropAtPendingPoints();

	/** Match the length of ActiveDrops to SpawnPoints, and keep what it already holds. */
	void SyncOccupancy();

	/** Set the warning places and show them on this machine. Server only. */
	void SetWarningTransforms(const TArray<FTransform>& InTransforms);

	UFUNCTION()
	void OnRep_WarningTransforms();

	/** Spawn a local warning marker at every warning place, and remove the old ones. Runs on every machine. */
	void RefreshWarningMarkers();

	void ClearWarningMarkers();

	/** Drop on each spawn point, by index. A picked up or expired drop is gone, so its point is free again. */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ABonusDrop>> ActiveDrops;

	/** Points this round picked and not dropped on yet. Server only. */
	TArray<int32> PendingIndices;

	/** Floor places under the picked points while the warning is up. Empty otherwise. */
	UPROPERTY(ReplicatedUsing = OnRep_WarningTransforms)
	TArray<FTransform> WarningTransforms;

	/** Markers this machine spawned for WarningTransforms. They do not replicate. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> WarningMarkers;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle WarningTimerHandle;

public:

	//~ Begin AActor Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

protected:

	//~ Begin AActor Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};
