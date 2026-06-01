// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "PresentSpawner.generated.h"

class APresent;
class ATargetPoint;
class UAkAudioEvent;
class UAkComponent;

UCLASS()
class TROMBONERUMBLE_API APresentSpawner : public AGimmickBase
{
	GENERATED_BODY()

public:
	APresentSpawner();

	virtual void Activate() override;
	virtual void Deactivate() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SantaMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UAkComponent> AkComponent;

	/** 스폰할 선물 클래스 */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "선물 클래스"))
	TSubclassOf<APresent> PresentClass;

	/** 산타 시작/재스폰 지점들 (매번 랜덤 선택) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "순찰 시작 지점 목록 (PointA)"))
	TArray<TObjectPtr<ATargetPoint>> PatrolPointsA;

	/** 산타 중간 경유지 (선물 드롭 지점, 매번 랜덤 선택) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "중간 경유 지점 목록 (선물 드롭)"))
	TArray<TObjectPtr<ATargetPoint>> MidPatrolPoints;

	/** 산타 종착 지점들 (매번 랜덤 선택) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "순찰 종착 지점 목록 (PointB)"))
	TArray<TObjectPtr<ATargetPoint>> PatrolPointsB;

	/** 산타 이동 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "산타 이동 속도 (cm/s)"))
	float SantaSpeed = 400.f;

	/** PointB 도착 후 PointA 재스폰까지 대기 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "재스폰 대기 시간 (초)"))
	float RespawnDelay = 10.f;

	/** DropPoint 감지 반경 (cm) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "드롭 감지 반경 (cm)"))
	float DropTriggerRadius = 150.f;

	/** 선물 획득 시 부여할 점수 */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "선물 획득 점수"))
	int32 PresentBonusScore = 300;

	/** 산타 이동 중 재생되는 루프 사운드 시작 이벤트 */
	UPROPERTY(EditDefaultsOnly, Category = "Present|Audio")
	TObjectPtr<UAkAudioEvent> JingleStartEvent;

	/** 산타 정지 시 루프 사운드 중지 이벤트 */
	UPROPERTY(EditDefaultsOnly, Category = "Present|Audio")
	TObjectPtr<UAkAudioEvent> JingleStopEvent;

	/** 선물 드롭 시 재생되는 "호호호!" 사운드 이벤트 */
	UPROPERTY(EditDefaultsOnly, Category = "Present|Audio")
	TObjectPtr<UAkAudioEvent> HoHoHoEvent;

private:
	void StartPatrol();
	void OnReachedMidPoint();
	void OnReachedEndPoint();
	void RespawnSanta();
	void SpawnPresent(const FVector& DropLocation);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayJingleStart();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayJingleStop();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHoHoHo();

	bool bIsPatrolling = false;
	bool bMidPointReached = false;
	FVector PatrolTargetLocation = FVector::ZeroVector;
	FVector NextPatrolTargetLocation = FVector::ZeroVector;

	FTimerHandle RespawnTimerHandle;
};
