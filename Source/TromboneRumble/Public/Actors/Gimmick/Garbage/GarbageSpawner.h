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
	// 서버에서 1회 쓰레기 스폰
	UFUNCTION(BlueprintCallable, Category = "GarbageSpawner")
	void SpawnGarbageOnce_Server();

	UFUNCTION(BlueprintCallable, Category = "GarbageSpawner")
	void StartAutoSpawn_Server();

	UFUNCTION(BlueprintCallable, Category = "GarbageSpawner")
	void StopAutoSpawn_Server();

protected:
	TSubclassOf<AGarbageBase> PickRandomGarbageClass() const;

	// SpawnPoint 배열에서 랜덤 위치 선택
	bool PickRandomSpawnTransform(FTransform& OutTransform) const;

	AActor* PickRandomPlayerPawn() const;

	// 실제 스폰 및 발사 처리
	void SpawnAndThrow_Server(const FTransform& SpawnTransform, AActor* TargetPawn);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GarbageSpawner|Garbage")
	TArray<TSubclassOf<AGarbageBase>> GarbageClasses;

	// 월드에 배치된 SpawnPoint Actor 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GarbageSpawner|Spawn")
	TArray<TObjectPtr<AActor>> SpawnPointActors;

	// 타겟 위치에 적용할 랜덤 오프셋 반경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GarbageSpawner|Target")
	float TargetRandomRadius = 80.f;

	// 자동 스폰 간격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GarbageSpawner|Auto")
	float SpawnInterval = 1.5f;

	// BeginPlay 시 자동 스폰 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GarbageSpawner|Auto")
	bool bAutoStart = true;

private:
	FTimerHandle AutoSpawnTimer;

};
