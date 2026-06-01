// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/PuddleTrap/WaterDropSpawner.h"
#include "Components/BoxComponent.h"
#include "Actors/Gimmick/PuddleTrap/WaterDrop.h"
#include "Engine/TargetPoint.h"

AWaterDropSpawner::AWaterDropSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AWaterDropSpawner::Activate()
{
	Super::Activate();
	
	if (HasAuthority() && SpawnInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ThisClass::SpawnOneDrop,
			SpawnInterval,
			true);
	}
}

void AWaterDropSpawner::Deactivate()
{
	Super::Deactivate();

	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		SpawnTimerHandle.Invalidate();
	}
}

void AWaterDropSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AWaterDropSpawner::SpawnOneDrop()
{
	if (!HasAuthority()) return;
	if (!WaterDropClass || SpawnPoints.IsEmpty()) return;

	const int32 RandIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
	TObjectPtr<ATargetPoint> ChosenPoint = SpawnPoints[RandIndex];

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AWaterDrop>(
		WaterDropClass,
		ChosenPoint->GetActorLocation(),
		ChosenPoint->GetActorRotation(),
		Params);
}

