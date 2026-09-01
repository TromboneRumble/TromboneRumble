// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameFramework/Actor.h"
#include "DrunkardSpawner.generated.h"

class ADrunkardNPC;
class UDrunkardDataAsset;

/** ADrunkardSpawner
 *
 * 재즈바 레벨에 배치하는 취객 NPC 스포너. Server Only.
 * - 최초 스폰 시간 / 반복 스폰 주기 관리 (수치는 UDrunkardDataAsset)
 * - 동시 1명 제한 (NPC 존재/퇴장 중 추가 스폰 없음)
 * - 문 목록 보유: 스폰은 랜덤 문, 퇴장은 종료 시점 최근접 문
 */
UCLASS()
class TROMBONERUMBLE_API ADrunkardSpawner : public AGimmickBase
{
	GENERATED_BODY()

public:
	ADrunkardSpawner();

	/** 퇴장용 최근접 문 조회. 문이 없으면 nullptr */
	AActor* FindClosestDoor(const FVector& Location) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "취객 데이터"))
	TObjectPtr<UDrunkardDataAsset> DrunkardData;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "취객 NPC 클래스"))
	TSubclassOf<ADrunkardNPC> NPCClass;

	/** 취객이 드나드는 문 액터들. 레벨에서 지정 */
	UPROPERTY(EditInstanceOnly, Category = "Config", meta = (DisplayName = "문 목록"))
	TArray<TObjectPtr<AActor>> Doors;

private:
	void TrySpawnNPC();

	UFUNCTION()
	void HandleNPCDestroyed(AActor* DestroyedActor);

	TWeakObjectPtr<ADrunkardNPC> ActiveNPC;

	FTimerHandle SpawnTimerHandle;

	//~ Begin AActor Interface
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface
};
