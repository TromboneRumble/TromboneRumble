// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "PresentSpawner.generated.h"

class APresent;
class ATargetPoint;

UCLASS()
class TROMBONERUMBLE_API APresentSpawner : public AGimmickBase
{
	GENERATED_BODY()

public:
	APresentSpawner();

	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/** 스폰할 선물 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Present|Config", meta = (DisplayName = "선물 클래스"))
	TSubclassOf<APresent> PresentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Present|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

	/** 선물이 생성되는 시간 간격 (초)
	 * 값이 작을수록 선물이 더 자주 생성되어 떨어집니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Present|Config", meta = (DisplayName = "선물 생성 간격"))
	float SpawnInterval = 10.f;

	/** 선물 획득 시 부여할 점수 */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "선물 획득 점수"))
	int32 PresentBonusScore = 300;

	FTimerHandle SpawnTimerHandle;

	void SpawnOneDrop();
};
