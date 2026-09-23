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
	virtual void ForceTrigger() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Config")
	TSubclassOf<AWaterDrop> WaterDropClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

	// 기획 수치(생성 간격, 웅덩이 확장/유지/소멸 시간)는 UWaterDropGimmickConfig 에 있다. 빙판(Ice)은 UIceDropGimmickConfig

	FTimerHandle SpawnTimerHandle;

	void SpawnOneDrop();
};