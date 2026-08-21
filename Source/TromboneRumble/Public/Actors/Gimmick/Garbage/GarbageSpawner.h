// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameFramework/Actor.h"
#include "GarbageSpawner.generated.h"

UENUM(BlueprintType)
enum class EGarbageTargetingMode : uint8
{
	Random UMETA(DisplayName = "Random Player"),
	ScoreWeighted UMETA(DisplayName = "Score Weighted (Higher Rank = Higher Chance)")
};

class AGarbageBase;

UCLASS()
class TROMBONERUMBLE_API AGarbageSpawner : public AGimmickBase
{
	GENERATED_BODY()
	
public:	
	AGarbageSpawner();
	
	virtual void Activate() override;

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
	UPROPERTY(EditAnywhere, Category = "Gimmick|Garbage")
	TArray<TSubclassOf<AGarbageBase>> GarbageClasses;

	UPROPERTY(EditAnywhere, Category = "Gimmick|Spawn")
	TArray<TObjectPtr<AActor>> SpawnPointActors;
	
	/** 쓰레기가 목표 지점(플레이어 위치)에 정확히 떨어지지 않고 흩어지는 무작위 반경 (cm) */
	UPROPERTY(EditAnywhere, Category = "Gimmick|Config|Target", meta = (DisplayName = "타겟팅 오차 반경"))
	float TargetRandomRadius = 80.f;
    
	/** 자동 생성 시 다음 생성까지 걸리는 최소 대기 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Gimmick|Config|Auto", meta = (DisplayName = "최소 스폰 간격"))
	float SpawnIntervalMin = 5.0f;
    
	/** 자동 생성 시 다음 생성까지 걸리는 최대 대기 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Gimmick|Config|Auto", meta = (DisplayName = "최대 스폰 간격"))
	float SpawnIntervalMax = 10.0f;

	/** 게임 시작 시 스포너가 자동으로 동작을 시작할지 여부 */
	UPROPERTY(EditAnywhere, Category = "Gimmick|Config|Auto", meta = (DisplayName = "시작 시 자동 실행"))
	bool bAutoStart = true;

	/** 쓰레기를 누구에게 던질 것인지 결정하는 방식입니다. 
	 * 랜덤: 모든 플레이어 대상
	 * 점수 가중치: 순위가 높을수록(점수가 높을수록) 더 자주 공격받음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Config|Target", meta = (DisplayName = "대상 타겟팅 방식"))
	EGarbageTargetingMode TargetingMode = EGarbageTargetingMode::ScoreWeighted;

private:
	FTimerHandle AutoSpawnTimer;
	
	// For Debugging & Cheat
public:
	UFUNCTION(Server, Reliable)
	void Server_SpawnGarbageForDebugging(const int32 Count);
};