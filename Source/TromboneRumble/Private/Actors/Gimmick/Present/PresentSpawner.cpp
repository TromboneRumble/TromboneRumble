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

	SyncActivePresentsSize();

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

void APresentSpawner::SyncActivePresentsSize()
{
	// Empty()가 아니라 SetNum()인 이유: AGimmickManager가 음악 큐마다
	// DeactivateAllGimmicks(); ActivateAllGimmicks();를 호출하므로,
	// 여기서 배열을 비우면 재활성화 직후 점유 정보가 날아가 겹쳐 스폰된다
	if (ActivePresents.Num() != SpawnPoints.Num())
	{
		ActivePresents.SetNum(SpawnPoints.Num());
	}
}

void APresentSpawner::SpawnOneDrop()
{
	if (!HasAuthority()) return;
	if (!PresentClass || SpawnPoints.IsEmpty()) return;

	SyncActivePresentsSize();

	// 아직 선물이 없는(= 이미 획득되었거나 한 번도 안 나온) 지점만 후보로 삼는다.
	// 낙하 중인 선물도 점유로 간주해야 공중에서 겹치지 않는다.
	TArray<int32> FreeIndices;
	FreeIndices.Reserve(SpawnPoints.Num());
	for (int32 Index = 0; Index < SpawnPoints.Num(); ++Index)
	{
		if (SpawnPoints[Index] && !ActivePresents[Index].IsValid())
		{
			FreeIndices.Add(Index);
		}
	}

	// 모든 지점이 점유됨 → 아무것도 하지 않고 다음 주기를 기다린다 (타이머는 looping)
	if (FreeIndices.IsEmpty()) return;

	const int32 ChosenIndex = FreeIndices[FMath::RandRange(0, FreeIndices.Num() - 1)];
	TObjectPtr<ATargetPoint> ChosenPoint = SpawnPoints[ChosenIndex];

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
		ActivePresents[ChosenIndex] = NewPresent;
	}
}
