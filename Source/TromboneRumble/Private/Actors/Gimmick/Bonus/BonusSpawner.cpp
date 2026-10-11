// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Bonus/BonusSpawner.h"
#include "Actors/Gimmick/Bonus/BonusDrop.h"
#include "Data/Gimmick/BonusGimmickConfig.h"
#include "Engine/TargetPoint.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ABonusSpawner::ABonusSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Every player sees the warning markers, wherever they stand
	bAlwaysRelevant = true;

	// The Blueprint of each level sets its own type, such as the docking port
	GimmickType = EGimmickType::Present;
}

void ABonusSpawner::Activate()
{
	Super::Activate();

	SyncOccupancy();

	const float SpawnInterval = GetConfig<UBonusGimmickConfig>().SpawnInterval;
	if (HasAuthority() && SpawnInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::StartDrop, SpawnInterval, true);
	}
}

void ABonusSpawner::Deactivate()
{
	Super::Deactivate();

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);

	// A warning that never drops would leave its markers up
	if (HasAuthority())
	{
		PendingIndices.Reset();
		SetWarningTransforms({});
	}
}

void ABonusSpawner::ForceTrigger()
{
	StartDrop();
}

void ABonusSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);
	ClearWarningMarkers();

	Super::EndPlay(EndPlayReason);
}

void ABonusSpawner::SyncOccupancy()
{
	// SetNum and not Empty: a restart from the settings panel calls Deactivate then Activate,
	// and emptying here would drop onto points that still hold a drop
	if (ActiveDrops.Num() != SpawnPoints.Num())
	{
		ActiveDrops.SetNum(SpawnPoints.Num());
	}
}

void ABonusSpawner::StartDrop()
{
	if (!HasAuthority()) return;

	// The last warning is still up, so this round is skipped
	if (!PendingIndices.IsEmpty()) return;

	const UBonusGimmickConfig& Config = GetConfig<UBonusGimmickConfig>();
	if (!Config.DropClass || SpawnPoints.IsEmpty()) return;

	SyncOccupancy();

	// A point with a drop on it, still falling or landed, is taken
	TArray<int32> FreeIndices;
	for (int32 Index = 0; Index < SpawnPoints.Num(); ++Index)
	{
		if (SpawnPoints[Index] && !ActiveDrops[Index].IsValid())
		{
			FreeIndices.Add(Index);
		}
	}

	// Too few free points means fewer drops this round
	const int32 WantedCount = FMath::RandRange(Config.MinDropCount, FMath::Max(Config.MinDropCount, Config.MaxDropCount));
	const int32 Count = FMath::Min(WantedCount, FreeIndices.Num());
	for (int32 Picked = 0; Picked < Count; ++Picked)
	{
		const int32 Slot = FMath::RandRange(0, FreeIndices.Num() - 1);
		PendingIndices.Add(FreeIndices[Slot]);
		FreeIndices.RemoveAtSwap(Slot);
	}
	if (PendingIndices.IsEmpty()) return;

	// No warning time means no warning. A zero timer would never fire
	if (Config.WarningDuration <= 0.f)
	{
		DropAtPendingPoints();
		return;
	}

	// The marker sits on the floor where the drop lands, not in the air where it starts
	TArray<FTransform> Transforms;
	for (const int32 Index : PendingIndices)
	{
		const ATargetPoint* Point = SpawnPoints[Index];
		FVector Ground = Point->GetActorLocation();
		ABonusDrop::FindGroundBelow(GetWorld(), Ground, Ground);
		Transforms.Add(FTransform(Point->GetActorRotation(), Ground));
	}
	SetWarningTransforms(Transforms);

	GetWorldTimerManager().SetTimer(WarningTimerHandle, this, &ThisClass::DropAtPendingPoints, Config.WarningDuration, false);
}

void ABonusSpawner::DropAtPendingPoints()
{
	if (!HasAuthority()) return;

	const TArray<int32> Indices = MoveTemp(PendingIndices);
	PendingIndices.Reset();
	SetWarningTransforms({});

	const UBonusGimmickConfig& Config = GetConfig<UBonusGimmickConfig>();
	if (!Config.DropClass) return;

	SyncOccupancy();

	for (const int32 Index : Indices)
	{
		const ATargetPoint* Point = SpawnPoints.IsValidIndex(Index) ? SpawnPoints[Index].Get() : nullptr;
		if (!Point) continue;

		const FTransform SpawnTransform(Point->GetActorRotation(), Point->GetActorLocation());
		ABonusDrop* Drop = GetWorld()->SpawnActorDeferred<ABonusDrop>(Config.DropClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Drop) continue;

		Drop->ApplyConfig(Config);
		Drop->FinishSpawning(SpawnTransform);

		ActiveDrops[Index] = Drop;
	}
}

void ABonusSpawner::SetWarningTransforms(const TArray<FTransform>& InTransforms)
{
	WarningTransforms = InTransforms;
	ForceNetUpdate();

	// OnRep never runs on the server, so the listen host shows its own markers here
	RefreshWarningMarkers();
}

void ABonusSpawner::OnRep_WarningTransforms()
{
	RefreshWarningMarkers();
}

void ABonusSpawner::RefreshWarningMarkers()
{
	ClearWarningMarkers();

	if (GetNetMode() == NM_DedicatedServer) return;

	const TSubclassOf<AActor> MarkerClass = GetConfig<UBonusGimmickConfig>().WarningMarkerClass;
	if (!MarkerClass) return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Every machine spawns its own markers from the replicated places, so the marker Blueprint does not replicate
	for (const FTransform& Transform : WarningTransforms)
	{
		if (AActor* Marker = GetWorld()->SpawnActor<AActor>(MarkerClass, Transform, Params))
		{
			WarningMarkers.Add(Marker);
		}
	}
}

void ABonusSpawner::ClearWarningMarkers()
{
	for (AActor* Marker : WarningMarkers)
	{
		if (IsValid(Marker))
		{
			Marker->Destroy();
		}
	}
	WarningMarkers.Reset();
}

void ABonusSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, WarningTransforms);
}
