// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorPoolSubsystem.generated.h"


USTRUCT()
struct FActorPool
{
	GENERATED_BODY()

	UPROPERTY() TSubclassOf<AActor> Class;
	UPROPERTY() TArray<TObjectPtr<AActor>> Inactive; // 비활성 큐
	UPROPERTY() TSet<TObjectPtr<AActor>> Active;     // 활성 집합
	int32 TotalSpawned = 0;
};


UCLASS()
class TROMBONERUMBLE_API UActorPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	// 사전 생성(프리웜)
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void Prewarm(TSubclassOf<AActor> ActorClass, int32 Count, const FTransform& SpawnTransform);

	// 꺼내기
	UFUNCTION(BlueprintCallable, Category = "Pool")
	AActor* Acquire(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

	// 반납
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void Release(AActor* Actor);

	// 풀 축소(오래 남은 비활성 객체 제거)
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void Shrink(TSubclassOf<AActor> ActorClass, int32 MaxInactive);

	// 전부 반납/정리
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void ReleaseAll();

protected:
	virtual void Deinitialize() override;

private:
	TMap<TSubclassOf<AActor>, FActorPool> Pools;

	AActor* SpawnPooledActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, bool bDeactivateImmediately);
	void ActivateForUse(AActor* Actor, const FTransform& SpawnTransform);
	void DeactivateForPool(AActor* Actor);

	UFUNCTION() void OnActorDestroyed(AActor* Actor);
};
