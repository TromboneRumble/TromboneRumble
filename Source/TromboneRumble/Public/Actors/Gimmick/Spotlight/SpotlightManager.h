// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickSpawnerBase.h"
#include "SpotlightManager.generated.h"

class ATargetPoint;
class ASpotlightZone;

UCLASS(Abstract)
class TROMBONERUMBLE_API ASpotlightManager : public AGimmickSpawnerBase
{
	GENERATED_BODY()
	
public:
	ASpotlightManager();

protected:
	//~ Begin AGimmickSpawnerBase Interface
	virtual const USpawnerGimmickConfig& GetSpawnerConfig() const override;
	virtual void SpawnBatch(int32 Count) override;
	//~ End AGimmickSpawnerBase Interface

private:
	UFUNCTION()
	void OnSpotlightZoneDestroyed(AActor* DestroyedActor);
	
	bool IsSpawnPointOccupied(const FVector& Location) const;
	
    UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Config")
    TSubclassOf<ASpotlightZone> SpotlightZoneClass;

    /** 스포트라이트가 생성될 수 있는 후보 위치들의 리스트입니다. (TargetPoint 사용) */
    UPROPERTY(EditAnywhere, Category = "Gimmick|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
    TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

    // 기획 수치(일반/피버 스폰 규칙, 존의 예고/발동/사라지는 시간, 성공 보너스 점수)는 USpotlightGimmickConfig 에 있다
	
	UPROPERTY()
	TSet<TObjectPtr<ASpotlightZone>> ActiveSpotlightZones;
	
	// For Debugging & Cheat
	
public:
	UFUNCTION(Server, Reliable)
	void Server_TriggerAllSpotlightSpawn();
};
