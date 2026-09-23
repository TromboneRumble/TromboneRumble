// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Drunkard/DrunkardSpawner.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "Data/Gimmick/DrunkardGimmickConfig.h"
#include "Engine/Engine.h"
#include "Engine/TargetPoint.h"
#include "Utilities/TromboneLogs.h"


#if !UE_BUILD_SHIPPING
extern TAutoConsoleVariable<int32> CVarDrunkardDebug;
#endif

ADrunkardSpawner::ADrunkardSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	GimmickType = EGimmickType::Drunkard;
}

void ADrunkardSpawner::Activate()
{
	const bool bWasActive = IsActive();

	Super::Activate();

	if (!bWasActive && HasAuthority())
	{
		const float Delay = GetConfig<UDrunkardGimmickConfig>().InitialSpawnDelay;
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::TrySpawnNPC, Delay, false);

		UE_LOG(LogGimmick, Log, TEXT("Drunkard spawner started. First spawn in %.1fs"), Delay);
	}
}

void ADrunkardSpawner::ForceTrigger()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	TrySpawnNPC();
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
			? FString::Printf(TEXT("[취객 스포너] 다음 스폰까지 %.1fs (동선 %d개)"), Remaining, Routes.Num())
			: TEXT("[취객 스포너] 스폰 타이머 비활성");
	}
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Orange, Text);
#endif
}

void ADrunkardSpawner::TrySpawnNPC()
{
	if (!HasAuthority() || ActiveNPC.IsValid() || !NPCClass) return;

	// Random entrance. Without one the spawner's own transform is the entrance
	FDrunkardRoute Route;
	TArray<const FDrunkardRoute*> ValidRoutes;
	for (const FDrunkardRoute& Candidate : Routes)
	{
		if (Candidate.SpawnPoint) ValidRoutes.Add(&Candidate);
	}
	if (!ValidRoutes.IsEmpty())
	{
		Route = *ValidRoutes[FMath::RandRange(0, ValidRoutes.Num() - 1)];
	}

	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();
	if (Route.SpawnPoint)
	{
		SpawnLocation = Route.SpawnPoint->GetActorLocation();
		// Faces the stop point so the walk in goes straight forward
		const FVector ToStop = Route.StopPoint ? (Route.StopPoint->GetActorLocation() - SpawnLocation).GetSafeNormal2D() : FVector::ZeroVector;
		SpawnRotation = ToStop.IsNearlyZero() ? Route.SpawnPoint->GetActorRotation() : ToStop.Rotation();
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
		State->BeginEntering(Route);
	}

	OnDrunkardSpawned.Broadcast(NPC, Route.Door);
}

void ADrunkardSpawner::HandleNPCDestroyed(AActor* DestroyedActor)
{
	OnDrunkardDespawned.Broadcast(Cast<ADrunkardNPC>(DestroyedActor));

	ActiveNPC = nullptr;

	if (!IsActive()) return;

	const float Interval = GetConfig<UDrunkardGimmickConfig>().RespawnInterval;
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::TrySpawnNPC, Interval, false);
}

void ADrunkardSpawner::HandleNPCCaptureSucceeded(ADefaultTromboneCharacter* Target)
{
	OnDrunkardCaptureSucceeded.Broadcast(ActiveNPC.Get(), Target);
}
