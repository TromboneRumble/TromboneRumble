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

	/**
	 * SpawnPoints와 인덱스가 1:1로 대응하는 점유 현황.
	 * 선물은 획득 시 Destroy()되므로 약참조가 자동으로 무효화되어 지점이 해제된다.
	 */
	UPROPERTY()
	TArray<TWeakObjectPtr<APresent>> ActivePresents;

	FTimerHandle SpawnTimerHandle;

	void SpawnOneDrop();

	/** ActivePresents 길이를 SpawnPoints에 맞춘다 (기존 점유 정보는 보존) */
	void SyncActivePresentsSize();
};
