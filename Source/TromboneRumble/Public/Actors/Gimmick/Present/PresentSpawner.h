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
	virtual void ForceTrigger() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// 기획 수치(선물 클래스, 생성 간격, 획득 점수)는 UPresentGimmickConfig 에 있다

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Present|Config", meta = (DisplayName = "스폰 가능 지점 목록"))
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

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
