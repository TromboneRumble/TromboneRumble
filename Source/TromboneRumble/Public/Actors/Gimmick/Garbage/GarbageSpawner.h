// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameFramework/Actor.h"
#include "GarbageSpawner.generated.h"

class AGarbageBase;

UCLASS()
class TROMBONERUMBLE_API AGarbageSpawner : public AGimmickBase
{
	GENERATED_BODY()
	
public:	
	AGarbageSpawner();
	
	virtual void Activate() override;
	virtual void ForceTrigger() override;
#if WITH_EDITOR
	virtual void GetSettingsObjects(TArray<UObject*>& OutObjects) const override;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void Server_SpawnGarbageOnce();
	void Server_StartAutoSpawn();
	void Server_StopAutoSpawn();

	UFUNCTION()
	void StartAutoSpawnFromMusicCue(FName CueName);

protected:
	void ScheduleNextSpawn();
	void SpawnAndReschedule();

	TSubclassOf<AGarbageBase> PickRandomGarbageClass() const;
	bool PickRandomSpawnTransform(FTransform& OutTransform) const;
	AActor* PickTargetPawn() const;
	AActor* PickRandomPlayerPawn() const;
	AActor* PickScoreWeightedTargetPawn() const;

	void SpawnAndThrow_Server(const FTransform& SpawnTransform, AActor* TargetPawn);

protected:
	// 기획 수치(쓰레기 종류, 투척 간격, 오차 반경, 타겟팅 방식)는 UGarbageGimmickConfig 에 있다

	UPROPERTY(EditAnywhere, Category = "Gimmick|Spawn")
	TArray<TObjectPtr<AActor>> SpawnPointActors;

private:
	FTimerHandle AutoSpawnTimer;
	
	// For Debugging & Cheat
public:
	UFUNCTION(Server, Reliable)
	void Server_SpawnGarbageForDebugging(const int32 Count);
};