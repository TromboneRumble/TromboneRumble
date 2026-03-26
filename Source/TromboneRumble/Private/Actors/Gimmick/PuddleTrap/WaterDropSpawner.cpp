// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/PuddleTrap/WaterDropSpawner.h"
#include "Components/BoxComponent.h"
#include "Actors/Gimmick/PuddleTrap/WaterDrop.h"

AWaterDropSpawner::AWaterDropSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SetRootComponent(SpawnBox);
	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

void AWaterDropSpawner::SpawnOneDrop()
{
	if (!HasAuthority()) return;
	if (!WaterDropClass || !SpawnBox) return;

	const FVector Origin = SpawnBox->GetComponentLocation();
	const FVector Extent = SpawnBox->GetScaledBoxExtent();

	// Box 안에서 랜덤 X,Y, 맨 위 Z에서 스폰
	const float RandX = FMath::FRandRange(-Extent.X, Extent.X);
	const float RandY = FMath::FRandRange(-Extent.Y, Extent.Y);
	const float SpawnZ = Origin.Z + Extent.Z; // 상단 면

	const FVector SpawnLocation = FVector(
		Origin.X + RandX,
		Origin.Y + RandY,
		SpawnZ);

	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AWaterDrop>(
		WaterDropClass,
		FTransform(SpawnRotation, SpawnLocation),
		Params);
}

