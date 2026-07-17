// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/Present/PresentSpawner.h"
#include "Actors/Gimmick/Present/Present.h"
#include "Engine/TargetPoint.h"

APresentSpawner::APresentSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GimmickType = EGimmickType::Present;
}

void APresentSpawner::Activate()
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

void APresentSpawner::Deactivate()
{
	Super::Deactivate();

	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		SpawnTimerHandle.Invalidate();
	}
}

void APresentSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APresentSpawner::SpawnOneDrop()
{
	if (!HasAuthority()) return;
	if (!PresentClass || SpawnPoints.IsEmpty()) return;

	const int32 RandIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
	TObjectPtr<ATargetPoint> ChosenPoint = SpawnPoints[RandIndex];
	if (!ChosenPoint) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (APresent* NewPresent = GetWorld()->SpawnActor<APresent>(
		PresentClass,
		ChosenPoint->GetActorLocation(),
		ChosenPoint->GetActorRotation(),
		Params))
	{
		NewPresent->BonusScore = PresentBonusScore;
	}
}
