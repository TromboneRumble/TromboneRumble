// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameFramework/Actor.h"
#include "WaterDropSpawner.generated.h"

class ATargetPoint;
class AWaterDrop;

UCLASS(Abstract)
class TROMBONERUMBLE_API AWaterDropSpawner : public AGimmickBase
{
	GENERATED_BODY()
	
public:	
	AWaterDropSpawner();
	
	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Config")
	TSubclassOf<AWaterDrop> WaterDropClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

	/** 물방울이 생성되는 시간 간격 (초) 
	 * 값이 작을수록 물방울이 더 자주 생성되어 떨어집니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Config", meta = (DisplayName = "물방울 생성 간격"))
	float SpawnInterval = 10.f;

	FTimerHandle SpawnTimerHandle;

	void SpawnOneDrop();
};