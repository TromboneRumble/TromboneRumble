// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/ActorPoolSubsystem.h"

#include "Interfaces/Poolable.h"
#include "Kismet/GameplayStatics.h"

static void SetInActive(AActor* Actor, bool bInActive)
{
	if (!Actor) return;
	Actor->SetActorHiddenInGame(bInActive);
	Actor->SetActorTickEnabled(!bInActive);
	Actor->SetActorEnableCollision(!bInActive);
}

void UActorPoolSubsystem::Prewarm(TSubclassOf<AActor> ActorClass, int32 Count, const FTransform& SpawnTransform)
{
	if (!ActorClass || Count <= 0) return;
	FActorPool& Pool = Pools.FindOrAdd(ActorClass);
	Pool.Class = ActorClass;

	Pool.Inactive.Reserve(Pool.Inactive.Num() + Count);

	for (int32 i = 0; i < Count; ++i)
	{
		if (AActor* A = SpawnPooledActor(ActorClass, SpawnTransform, true))
		{
			Pool.Inactive.Add(A);
			Pool.TotalSpawned++;
		}
	}
}

AActor* UActorPoolSubsystem::Acquire(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
	if (!ActorClass) return nullptr;

	FActorPool& Pool = Pools.FindOrAdd(ActorClass);
	// ActorClass가 처음 등장한 경우 클래스 정보 설정
	Pool.Class = ActorClass;

	AActor* PooledActor = nullptr;

	// 비활성 재사용
	while (Pool.Inactive.Num() > 0 && !PooledActor)
	{
		AActor* Candidate = Pool.Inactive.Pop(EAllowShrinking::No);
		if (IsValid(Candidate))
		{
			PooledActor = Candidate;
		}
	}

	// 없으면 새로 생성
	if (!PooledActor)
	{
		PooledActor = SpawnPooledActor(ActorClass, SpawnTransform, true);
		Pool.TotalSpawned++;
	}

	if (PooledActor)
	{
		ActivateForUse(PooledActor, SpawnTransform);
		Pool.Active.Add(PooledActor);
	}
	return PooledActor;
}

void UActorPoolSubsystem::Release(AActor* Actor)
{
	if (!IsValid(Actor)) return;

	// 어떤 풀인지 찾기
	TSubclassOf<AActor> KeyClass = Actor->GetClass();
	FActorPool* Pool = Pools.Find(KeyClass);

	if (!Pool)
	{
		// 풀 밖에서 온 경우라도 안전하게 비활성 처리
		DeactivateForPool(Actor);
		return;
	}

	if (Pool->Active.Remove(Actor) > 0)
	{
		DeactivateForPool(Actor);
		Pool->Inactive.Add(Actor);
	}
}

void UActorPoolSubsystem::Shrink(TSubclassOf<AActor> ActorClass, int32 MaxInactive)
{
	if (!*ActorClass) return;
	if (FActorPool* Pool = Pools.Find(ActorClass))
	{
		while (Pool->Inactive.Num() > MaxInactive)
		{
			if (AActor* A = Pool->Inactive.Pop())
			{
				if (IsValid(A)) A->Destroy();
			}
		}
	}
}

void UActorPoolSubsystem::ReleaseAll()
{
	for (auto& KV : Pools)
	{
		FActorPool& Pool = KV.Value;
		for (AActor* A : Pool.Active)
		{
			if (IsValid(A)) DeactivateForPool(A);
		}
		Pool.Inactive.Append(Pool.Active.Array());
		Pool.Active.Reset();
	}
}

void UActorPoolSubsystem::Deinitialize()
{
	ReleaseAll();
	Super::Deinitialize();
}

AActor* UActorPoolSubsystem::SpawnPooledActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform,
	bool bDeactivateImmediately)
{
	UWorld* World = GetWorld();
	if (!World || !ActorClass) return nullptr;

	AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(ActorClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!SpawnedActor) return nullptr;

	// 생성 직후 기본은 비활성
	if (bDeactivateImmediately)
	{
		SetInActive(SpawnedActor, true);
	}

	UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);
	SpawnedActor->OnDestroyed.AddDynamic(this, &ThisClass::OnActorDestroyed);
	return SpawnedActor;
}

void UActorPoolSubsystem::ActivateForUse(AActor* Actor, const FTransform& SpawnTransform)
{
	Actor->SetActorTransform(SpawnTransform);
	SetInActive(Actor, false);

	if (Actor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
	{
		IPoolable::Execute_OnTakenFromPool(Actor);
	}
}

void UActorPoolSubsystem::DeactivateForPool(AActor* Actor)
{
	if (!Actor) return;

	// 사용자 훅
	if (Actor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
	{
		IPoolable::Execute_OnReturnToPool(Actor);
	}

	// 비활성화
	SetInActive(Actor, true);
}



void UActorPoolSubsystem::OnActorDestroyed(AActor* Actor)
{
	if (!Actor) return;
	auto It = Pools.CreateIterator();
	while (It)
	{
		FActorPool& Pool = It.Value();
		Pool.Active.Remove(Actor);
		Pool.Inactive.Remove(Actor);
		++It;
	}
}
