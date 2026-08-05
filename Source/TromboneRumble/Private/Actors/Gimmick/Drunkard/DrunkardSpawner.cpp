// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Drunkard/DrunkardSpawner.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "Data/DrunkardDataAsset.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogDrunkard, Log, All);

#if !UE_BUILD_SHIPPING
extern TAutoConsoleVariable<int32> CVarDrunkardDebug; // 정의: DrunkardNPC.cpp
#endif

namespace
{
	// DrunkardData 미지정 시 폴백 (수치 튜닝은 데이터 에셋에서)
	constexpr float FallbackSpawnDelay = 20.f;
}

ADrunkardSpawner::ADrunkardSpawner()
{
	// 디버그 표시(Trombone.Drunkard.Debug) 전용 틱. 평상시엔 첫 분기에서 리턴한다
	PrimaryActorTick.bCanEverTick = true;
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

void ADrunkardSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	const float Delay = DrunkardData ? DrunkardData->InitialSpawnDelay : FallbackSpawnDelay;
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::TrySpawnNPC, Delay, false);
}

void ADrunkardSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void ADrunkardSpawner::TrySpawnNPC()
{
	if (!HasAuthority() || ActiveNPC.IsValid() || !NPCClass) return;

	// 스폰 위치 = 랜덤 문. 문이 지정되지 않았으면 스포너 위치 사용
	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	TArray<AActor*> ValidDoors;
	for (AActor* Door : Doors)
	{
		if (Door) ValidDoors.Add(Door);
	}
	if (!ValidDoors.IsEmpty())
	{
		const AActor* Door = ValidDoors[FMath::RandRange(0, ValidDoors.Num() - 1)];
		SpawnLocation = Door->GetActorLocation();
		SpawnRotation = Door->GetActorRotation();
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

	UE_LOG(LogDrunkard, Log, TEXT("%s 취객 스폰: %s (위치 %s)"), *GetName(), *NPC->GetName(), *SpawnLocation.ToCompactString());

	if (UDrunkardStateComponent* State = NPC->GetStateComponent())
	{
		State->BeginEntering();
	}
}

void ADrunkardSpawner::HandleNPCDestroyed(AActor* DestroyedActor)
{
	ActiveNPC = nullptr;

	const float Interval = DrunkardData ? DrunkardData->RespawnInterval : FallbackSpawnDelay;
	UE_LOG(LogDrunkard, Log, TEXT("%s 취객 소멸 — %.1fs 후 재스폰"), *GetName(), Interval);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::TrySpawnNPC, Interval, false);
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
