// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickSpawnerBase.h"
#include "GarbageSpawner.generated.h"

class AGarbageBase;

UCLASS()
class TROMBONERUMBLE_API AGarbageSpawner : public AGimmickSpawnerBase
{
	GENERATED_BODY()
	
public:
	AGarbageSpawner();
	
	/** Throw one garbage at one player. Server only. */
	void Server_SpawnGarbageOnce();

protected:
	//~ Begin AGimmickSpawnerBase Interface
	virtual const USpawnerGimmickConfig& GetSpawnerConfig() const override;
	virtual void SpawnBatch(int32 Count) override;
	//~ End AGimmickSpawnerBase Interface

	TSubclassOf<AGarbageBase> PickRandomGarbageClass() const;
	bool PickRandomSpawnTransform(FTransform& OutTransform) const;
	AActor* PickTargetPawn() const;
	AActor* PickRandomPlayerPawn() const;
	AActor* PickScoreWeightedTargetPawn() const;

	void SpawnAndThrow_Server(const FTransform& SpawnTransform, AActor* TargetPawn);

protected:
	// 기획 수치(투척 규칙, 쓰레기 종류, 오차 반경, 타겟팅 방식)는 UGarbageGimmickConfig 에 있다

	UPROPERTY(EditAnywhere, Category = "Gimmick|Spawn")
	TArray<TObjectPtr<AActor>> SpawnPointActors;

	// For Debugging & Cheat
public:
	UFUNCTION(Server, Reliable)
	void Server_SpawnGarbageForDebugging(const int32 Count);
};