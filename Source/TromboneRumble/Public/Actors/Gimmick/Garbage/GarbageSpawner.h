// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GarbageSpawner.generated.h"

class AGarbageBase;

UCLASS()
class TROMBONERUMBLE_API AGarbageSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AGarbageSpawner();

protected:
	virtual void BeginPlay() override;

public:
	void SpawnGarbageOnce_Server();
	void StartAutoSpawn_Server();
	void StopAutoSpawn_Server();

protected:
	TSubclassOf<AGarbageBase> PickRandomGarbageClass() const;
	bool PickRandomSpawnTransform(FTransform& OutTransform) const;
	AActor* PickRandomPlayerPawn() const;

	void SpawnAndThrow_Server(const FTransform& SpawnTransform, AActor* TargetPawn);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GarbageSpawner|Garbage")
	TArray<TSubclassOf<AGarbageBase>> GarbageClasses;

	UPROPERTY(EditDefaultsOnly, Category = "GarbageSpawner|Spawn")
	TArray<TObjectPtr<AActor>> SpawnPointActors;

	UPROPERTY(EditAnywhere, Category = "GarbageSpawner|Target")
	float TargetRandomRadius = 80.f;
	
	UPROPERTY(EditAnywhere, Category = "GarbageSpawner|Auto")
	float SpawnIntervalMin = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "GarbageSpawner|Auto")
	float SpawnIntervalMax = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "GarbageSpawner|Auto")
	bool bAutoStart = true;

private:
	FTimerHandle AutoSpawnTimer;
};