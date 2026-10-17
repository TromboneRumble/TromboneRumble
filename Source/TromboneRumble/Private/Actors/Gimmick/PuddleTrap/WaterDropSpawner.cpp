// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/PuddleTrap/WaterDropSpawner.h"
#include "Components/BoxComponent.h"
#include "Actors/Gimmick/PuddleTrap/WaterDrop.h"
#include "Data/Gimmick/WaterDropGimmickConfig.h"
#include "Engine/TargetPoint.h"

AWaterDropSpawner::AWaterDropSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

const USpawnerGimmickConfig& AWaterDropSpawner::GetSpawnerConfig() const
{
	// The WaterDrop and the IceDrop spawner share this class. GetConfig finds the entry by gimmick type, so both get their own settings
	return GetConfig<UWaterDropGimmickConfig>();
}

void AWaterDropSpawner::SpawnBatch(const int32 Count)
{
	if (!HasAuthority()) return;
	if (!WaterDropClass) return;

	TArray<const ATargetPoint*> FreePoints;
	for (const ATargetPoint* Point : SpawnPoints)
	{
		if (Point)
		{
			FreePoints.Add(Point);
		}
	}

	// Each drop of one batch takes a different point. Too few points means fewer drops
	for (int32 Picked = 0; Picked < Count && !FreePoints.IsEmpty(); ++Picked)
	{
		const int32 Slot = FMath::RandRange(0, FreePoints.Num() - 1);
		SpawnOneDrop(*FreePoints[Slot]);
		FreePoints.RemoveAtSwap(Slot);
	}
}

void AWaterDropSpawner::SpawnOneDrop(const ATargetPoint& Point)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AWaterDrop* Drop = GetWorld()->SpawnActor<AWaterDrop>(
		WaterDropClass,
		Point.GetActorLocation(),
		Point.GetActorRotation(),
		Params))
	{
		// This class serves both the WaterDrop and the IceDrop gimmick. The puddle needs to know which one made it
		Drop->SetGimmickType(GetGimmickType());
	}
}
