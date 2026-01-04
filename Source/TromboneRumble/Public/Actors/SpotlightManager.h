// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpotlightManager.generated.h"

class AInGameState;
class ATargetPoint;
class ASpotlightZone;

UCLASS(Abstract)
class TROMBONERUMBLE_API ASpotlightManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpotlightManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void CheckSpotlightStart(FName CueName);
	void TriggerSpotlightSpawn();
	
    UPROPERTY(EditDefaultsOnly, Category = "Spotlight")
    TSubclassOf<ASpotlightZone> SpotlightZoneClass;

    /** 스포트라이트가 생성될 수 있는 후보 위치들의 리스트입니다. (TargetPoint 사용) */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
    TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

    /** 전체 곡 진행률(0.0 ~ 1.0) 중 스포트라이트 기믹이 시작될 시점입니다. */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "기믹 시작 진행률"))
    float SpotlightStartTimePercent = 0.1f;
    
    /** 전체 곡 진행률(0.0 ~ 1.0) 중 피버 타임(강화 모드)이 시작될 시점입니다. */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "피버 타임 시작 진행률"))
    float FeverTimeStartPercent = 0.8f;

    /** [일반] 스포트라이트가 생성되는 최소 시간 간격 (초) */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Normal", meta = (DisplayName = "최소 스폰 간격 (일반)"))
    float MinSpawnInterval_Normal = 8.0f;

    /** [일반] 스포트라이트가 생성되는 최대 시간 간격 (초) */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Normal", meta = (DisplayName = "최대 스폰 간격 (일반)"))
    float MaxSpawnInterval_Normal = 15.0f;

    /** [일반] 한 번에 생성되는 최소 스포트라이트 개수 */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Normal", meta = (DisplayName = "최소 스폰 개수 (일반)"))
    int32 MinSpawnCount_Normal = 1;

    /** [일반] 한 번에 생성되는 최대 스포트라이트 개수 */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Normal", meta = (DisplayName = "최대 스폰 개수 (일반)"))
    int32 MaxSpawnCount_Normal = 2;

    /** [피버] 피버 타임 시 스포트라이트가 생성되는 최소 시간 간격 (초) */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Fever", meta = (DisplayName = "최소 스폰 간격 (피버)"))
    float MinSpawnInterval_Fever = 3.0f;

    /** [피버] 피버 타임 시 스포트라이트가 생성되는 최대 시간 간격 (초) */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Fever", meta = (DisplayName = "최대 스폰 간격 (피버)"))
    float MaxSpawnInterval_Fever = 6.0f;

    /** [피버] 피버 타임 시 한 번에 생성되는 최소 스포트라이트 개수 */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Fever", meta = (DisplayName = "최소 스폰 개수 (피버)"))
    int32 MinSpawnCount_Fever = 2;

    /** [피버] 피버 타임 시 한 번에 생성되는 최대 스포트라이트 개수 */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Fever", meta = (DisplayName = "최대 스폰 개수 (피버)"))
    int32 MaxSpawnCount_Fever = 4;

    /** 플레이어가 스포트라이트 구역에서 연주 성공 시 획득하는 기본 점수 보너스 */
    UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "성공 보너스 점수"))
    int32 SpotlightBonusScore = 300;
	
	FTimerHandle SpawnTimerHandle;
	bool bIsSpotlightActive = false;
	bool bIsFeverTime = false;
};
