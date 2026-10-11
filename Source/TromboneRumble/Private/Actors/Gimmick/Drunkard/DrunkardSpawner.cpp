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

	// The spawner has no timer. A sequence of the stage data calls ForceTrigger when it is the drunkard's turn
	if (!bWasActive && HasAuthority())
	{
		UE_LOG(LogGimmick, Log, TEXT("Drunkard spawner started. It waits for its turn in the sequence"));
	}
}

void ADrunkardSpawner::ForceTrigger()
{
	if (!HasAuthority()) return;

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
		Text = FString::Printf(TEXT("[취객 스포너] 차례를 기다리는 중 (동선 %d개)"), Routes.Num());
	}
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Orange, Text);
#endif
}

void ADrunkardSpawner::TrySpawnNPC()
{
	if (!HasAuthority() || ActiveNPC.IsValid()) return;

	// Nothing spawns without a class, and a sequence would wait forever for the end of this turn
	if (!NPCClass)
	{
		NotifyEventFinished();
		return;
	}

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

	// Also when the spawner is turned off, so a sequence waiting for this turn does not stall
	NotifyEventFinished();
}

void ADrunkardSpawner::HandleNPCCaptureSucceeded(ADefaultTromboneCharacter* Target)
{
	OnDrunkardCaptureSucceeded.Broadcast(ActiveNPC.Get(), Target);
}
