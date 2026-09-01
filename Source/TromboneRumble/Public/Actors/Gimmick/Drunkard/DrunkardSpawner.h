// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameFramework/Actor.h"
#include "DrunkardSpawner.generated.h"

class ADefaultTromboneCharacter;
class ADrunkardNPC;
class UDrunkardDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpawnerDrunkardSpawned, ADrunkardNPC*, NPC, AActor*, Door);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnerDrunkardDespawned, ADrunkardNPC*, NPC);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpawnerDrunkardCaptureSucceeded, ADrunkardNPC*, NPC, ADefaultTromboneCharacter*, Target);

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

	/** Called when a drunkard has spawned. Door is where it came out, may be null. Server only. */
	FOnSpawnerDrunkardSpawned OnDrunkardSpawned;

	/** Called when the drunkard is being destroyed. Do not hold on to the NPC. Server only. */
	FOnSpawnerDrunkardDespawned OnDrunkardDespawned;

	/** Called when the drunkard has captured a target holding an instrument. Server only. */
	FOnSpawnerDrunkardCaptureSucceeded OnDrunkardCaptureSucceeded;

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

	/** NPC 상태 컴포넌트의 포획 성공을 받아 NPC 정보를 붙여 재방송한다 */
	UFUNCTION()
	void HandleNPCCaptureSucceeded(ADefaultTromboneCharacter* Target);

	TWeakObjectPtr<ADrunkardNPC> ActiveNPC;

	FTimerHandle SpawnTimerHandle;

protected:
	
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
	
public:
	
	//~ Begin AActor Interface
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface
};
