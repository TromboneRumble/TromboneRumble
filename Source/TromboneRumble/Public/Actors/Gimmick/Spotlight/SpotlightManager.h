// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayTypes.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameFramework/Actor.h"
#include "SpotlightManager.generated.h"

class AInGameState;
class ATargetPoint;
class ASpotlightZone;

UCLASS(Abstract)
class TROMBONERUMBLE_API ASpotlightManager : public AGimmickBase
{
	GENERATED_BODY()
	
public:	
	ASpotlightManager();
	
	virtual void Activate() override;
	virtual	void Deactivate() override;
	virtual void ForceTrigger() override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UFUNCTION()
    void OnMusicCallbackReceived(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	UFUNCTION()
	void OnSpotlightZoneDestroyed(AActor* DestroyedActor);
	
	void TriggerSpotlightSpawn();
	bool IsSpawnPointOccupied(const FVector& Location) const;
	
    UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Config")
    TSubclassOf<ASpotlightZone> SpotlightZoneClass;

    /** 스포트라이트가 생성될 수 있는 후보 위치들의 리스트입니다. (TargetPoint 사용) */
    UPROPERTY(EditAnywhere, Category = "Gimmick|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
    TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

    // 기획 수치(일반/피버 스폰 간격과 개수, 존의 경고/활성/소멸 시간, 성공 보너스 점수)는 USpotlightGimmickConfig 에 있다
	
	UPROPERTY()
	TSet<TObjectPtr<ASpotlightZone>> ActiveSpotlightZones;
	
	FTimerHandle SpawnTimerHandle;
	bool bIsFeverTime = false;
	// For Debugging & Cheat
	
public:
	UFUNCTION(Server, Reliable)
	void Server_TriggerAllSpotlightSpawn();
};
