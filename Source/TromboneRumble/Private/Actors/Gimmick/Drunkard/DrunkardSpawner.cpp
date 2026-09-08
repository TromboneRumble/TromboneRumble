// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Drunkard/DrunkardSpawner.h"
#include "Actors/Gimmick/Drunkard/DrunkardDoorBreakerComponent.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "Data/DrunkardDataAsset.h"
#include "Engine/Engine.h"
#include "Utilities/TromboneLogs.h"


#if !UE_BUILD_SHIPPING
extern TAutoConsoleVariable<int32> CVarDrunkardDebug;
#endif

namespace
{
	constexpr float FallbackSpawnDelay = 20.f;
}

ADrunkardSpawner::ADrunkardSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	GimmickType = EGimmickType::Drunkard;
	DoorBreaker = CreateDefaultSubobject<UDrunkardDoorBreakerComponent>(TEXT("DoorBreaker"));
}

void ADrunkardSpawner::Activate()
{
	const bool bWasActive = IsActive();

	Super::Activate();

	if (!bWasActive && HasAuthority())
	{
		const float Delay = DrunkardData ? DrunkardData->InitialSpawnDelay : FallbackSpawnDelay;
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::TrySpawnNPC, Delay, false);

		UE_LOG(LogGimmick, Log, TEXT("Drunkard spawner started. First spawn in %.1fs"), Delay);
	}
}

void ADrunkardSpawner::Deactivate()
{
	if (HasAuthority())
	{
		if (ADrunkardNPC* NPC = ActiveNPC.Get())
		{
			if (UDrunkardStateComponent* State = NPC->GetStateComponent())
			{
				State->DespawnOwner();
			}
		}
		ActiveNPC = nullptr;
	}

	// Super clears the timers
	Super::Deactivate();
}

void ADrunkardSpawner::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if !UE_BUILD_SHIPPING
	if (!GEngine || !HasAuthority() || CVarDrunkardDebug.GetValueOnGameThread() == 0) return;

	FString Text;
	if (ActiveNPC.IsValid())
	{
		Text = FString::Printf(TEXT("[취객 스포너] NPC 활성: %s"), *ActiveNPC->GetName());
	}
	else
	{
		const float Remaining = GetWorldTimerManager().GetTimerRemaining(SpawnTimerHandle);
		Text = (Remaining >= 0.f)
			? FString::Printf(TEXT("[취객 스포너] 다음 스폰까지 %.1fs (문 %d개)"), Remaining, Doors.Num())
			: TEXT("[취객 스포너] 스폰 타이머 비활성");
	}
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Orange, Text);
#endif
}

void ADrunkardSpawner::TrySpawnNPC()
{
	if (!HasAuthority() || ActiveNPC.IsValid() || !NPCClass) return;

	// 스폰 위치 = 랜덤 문. 문이 지정되지 않았으면 스포너 위치 사용
	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	AActor* SpawnDoor = nullptr;
	TArray<AActor*> ValidDoors;
	for (AActor* Door : Doors)
	{
		if (Door) ValidDoors.Add(Door);
	}
	if (!ValidDoors.IsEmpty())
	{
		SpawnDoor = ValidDoors[FMath::RandRange(0, ValidDoors.Num() - 1)];

		const float Offset = DrunkardData ? DrunkardData->BehindDoorOffset : 150.f;
		SpawnLocation = SpawnDoor->GetActorLocation() - SpawnDoor->GetActorForwardVector() * Offset;

		const FVector ToDoor = SpawnDoor->GetActorLocation() - SpawnLocation;
		SpawnRotation = ToDoor.IsNearlyZero() ? SpawnDoor->GetActorRotation() : ToDoor.GetSafeNormal().Rotation();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ADrunkardNPC* NPC = GetWorld()->SpawnActor<ADrunkardNPC>(NPCClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!NPC)
	{
		HandleNPCDestroyed(nullptr);
		return;
	}

	NPC->SetOwningSpawner(this);
	NPC->OnDestroyed.AddDynamic(this, &ThisClass::HandleNPCDestroyed);
	ActiveNPC = NPC;

	if (UDrunkardStateComponent* State = NPC->GetStateComponent())
	{
		State->OnCaptureSucceeded.AddDynamic(this, &ThisClass::HandleNPCCaptureSucceeded);
		State->BeginEntering();
	}

	OnDrunkardSpawned.Broadcast(NPC, SpawnDoor);
}

void ADrunkardSpawner::HandleNPCDestroyed(AActor* DestroyedActor)
{
	OnDrunkardDespawned.Broadcast(Cast<ADrunkardNPC>(DestroyedActor));

	ActiveNPC = nullptr;

	if (!IsActive()) return;

	const float Interval = DrunkardData ? DrunkardData->RespawnInterval : FallbackSpawnDelay;
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::TrySpawnNPC, Interval, false);
}

void ADrunkardSpawner::HandleNPCCaptureSucceeded(ADefaultTromboneCharacter* Target)
{
	OnDrunkardCaptureSucceeded.Broadcast(ActiveNPC.Get(), Target);
}

AActor* ADrunkardSpawner::FindClosestDoor(const FVector& Location) const
{
	AActor* Closest = nullptr;
	double ClosestDistSq = TNumericLimits<double>::Max();

	for (AActor* Door : Doors)
	{
		if (!Door) continue;

		const double DistSq = FVector::DistSquared(Door->GetActorLocation(), Location);
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			Closest = Door;
		}
	}
	return Closest;
}
