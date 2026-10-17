// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickSpawnerBase.h"
#include "WaterDropSpawner.generated.h"

class ATargetPoint;
class AWaterDrop;

UCLASS(Abstract)
class TROMBONERUMBLE_API AWaterDropSpawner : public AGimmickSpawnerBase
{
	GENERATED_BODY()
	
public:
	AWaterDropSpawner();

protected:

	//~ Begin AGimmickSpawnerBase Interface
	virtual const USpawnerGimmickConfig& GetSpawnerConfig() const override;
	virtual void SpawnBatch(int32 Count) override;
	//~ End AGimmickSpawnerBase Interface

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Config")
	TSubclassOf<AWaterDrop> WaterDropClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

	// 기획 수치(스폰 규칙, 자국이 커지는/유지/사라지는 시간)는 UWaterDropGimmickConfig 에 있다. 빙판(IceDrop)은 UIceDropGimmickConfig

	/** Drop one water drop from this point. Server only. */
	void SpawnOneDrop(const ATargetPoint& Point);
};