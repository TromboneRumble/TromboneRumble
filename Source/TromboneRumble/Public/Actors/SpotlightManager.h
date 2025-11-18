// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpotlightManager.generated.h"

class AInGameState;
class ATargetPoint;
class ASpotlightZone;

UCLASS()
class TROMBONERUMBLE_API ASpotlightManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpotlightManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

private:
	void CheckSpotlightStart();
	void TriggerSpotlightSpawn();
	float GetCurrentSongProgress() const;
	
	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	TSubclassOf<ASpotlightZone> SpotlightZoneClass;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (ToolTip = "스포트라이트 기믹 시작 퍼센트"))
	float SpotlightStartTimePercent = 0.1f;
	
	UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (ToolTip = "피버 타임 시작 퍼센트")) // TODO : 리듬 게임의 피버 타임 데이터와 연동 필요
	float FeverTimeStartPercent = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Normal", meta = (ToolTip = "스포트라이트 존 최소 스폰 간격 (초 단위)"))
	float MinSpawnInterval_Normal = 8.0f;
	UPROPERTY(EditAnywhere, Category = "Spotlight|Normal", meta = (ToolTip = "스포트라이트 존 최대 스폰 간격 (초 단위)"))
	float MaxSpawnInterval_Normal = 15.0f;
	UPROPERTY(EditAnywhere, Category = "Spotlight|Normal", meta = (ToolTip = "스포트라이트 존 최소 스폰 개수"))
	int32 MinSpawnCount_Normal = 1;
	UPROPERTY(EditAnywhere, Category = "Spotlight|Normal", meta = (ToolTip = "스포트라이트 존 최대 스폰 개수"))
	int32 MaxSpawnCount_Normal = 2;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Fever", meta = (ToolTip = "피버 타임 시 스포트라이트 존 최소 스폰 간격 (초 단위)"))
	float MinSpawnInterval_Fever = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Spotlight|Fever", meta = (ToolTip = "피버 타임 시 스포트라이트 존 최대 스폰 간격 (초 단위)"))
	float MaxSpawnInterval_Fever = 6.0f;
	UPROPERTY(EditAnywhere, Category = "Spotlight|Fever", meta = (ToolTip = "피버 타임 시 스포트라이트 존 최소 스폰 개수"))
	int32 MinSpawnCount_Fever = 2;
	UPROPERTY(EditAnywhere, Category = "Spotlight|Fever", meta = (ToolTip = "피버 타임 시 스포트라이트 존 최대 스폰 개수"))
	int32 MaxSpawnCount_Fever = 4;
	
	UPROPERTY()
	TObjectPtr<AInGameState> CachedInGameState;
	
	FTimerHandle SpawnTimerHandle;
	bool bIsSpotlightActive = false;
};
