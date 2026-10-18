// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/EventGimmickBase.h"
#include "GameFramework/Actor.h"
#include "DrunkardSpawner.generated.h"

class ADefaultTromboneCharacter;
class ADrunkardNPC;
class ATargetPoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpawnerDrunkardSpawned, ADrunkardNPC*, NPC, AActor*, Door);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnerDrunkardDespawned, ADrunkardNPC*, NPC);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpawnerDrunkardCaptureSucceeded, ADrunkardNPC*, NPC, ADefaultTromboneCharacter*, Target);

USTRUCT(BlueprintType)
struct FDrunkardRoute
{
	GENERATED_BODY()

	/** Where it spawns and where it walks to when leaving. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "생성/퇴장 지점"))
	TObjectPtr<ATargetPoint> SpawnPoint = nullptr;

	/** Where it stops after coming through the door. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "정지 지점"))
	TObjectPtr<ATargetPoint> StopPoint = nullptr;

	/** Broken the frame the drunkard crosses it. Empty means no door effect, fine for a blockout */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "문"))
	TObjectPtr<AActor> Door = nullptr;
};

/**
 * ADrunkardSpawner brings the drunkard NPC into the jazz bar. Server only.
 * It has no timer of its own. A sequence of the stage data gives it a turn, and it spawns one drunkard on a random route.
 * The drunkard leaves through the spawn point of the route it came in by, and the turn ends when the drunkard is gone.
 *
 * Keep in mind that only one drunkard is out at a time, so a forced trigger while one is still out does nothing.
 * The numbers a designer tunes are in UDrunkardGimmickConfig.
 *
 * @see ADrunkardNPC
 * @see UDrunkardGimmickConfig
 */
UCLASS()
class TROMBONERUMBLE_API ADrunkardSpawner : public AEventGimmickBase
{
	GENERATED_BODY()

public:
	ADrunkardSpawner();

	/** Called when a drunkard has spawned. Door is where it came out, may be null. Server only. */
	FOnSpawnerDrunkardSpawned OnDrunkardSpawned;

	/** Called when the drunkard is being destroyed. Do not hold on to the NPC. Server only. */
	FOnSpawnerDrunkardDespawned OnDrunkardDespawned;

	/** Called when the drunkard has captured a target, by ragdoll or by stun. Server only. */
	FOnSpawnerDrunkardCaptureSucceeded OnDrunkardCaptureSucceeded;

protected:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "취객 NPC 클래스"))
	TSubclassOf<ADrunkardNPC> NPCClass;

	/** Every way into the bar. One is picked at random per spawn */
	UPROPERTY(EditInstanceOnly, Category = "Config", meta = (DisplayName = "동선 목록"))
	TArray<FDrunkardRoute> Routes;

	//~ Begin AEventGimmickBase Interface
	virtual const UEventGimmickConfig& GetEventConfig() const override;
	virtual void OnEventPhaseStarted(int32 PhaseIndex) override;
	virtual void OnEventStopped() override;
	virtual bool RunsPhaseTimers() const override { return false; }
	//~ End AEventGimmickBase Interface

private:
	void TrySpawnNPC();

	UFUNCTION()
	void HandleNPCDestroyed(AActor* DestroyedActor);

	/** NPC 상태 컴포넌트의 포획 성공을 받아 NPC 정보를 붙여 재방송한다 */
	UFUNCTION()
	void HandleNPCCaptureSucceeded(ADefaultTromboneCharacter* Target);

	TWeakObjectPtr<ADrunkardNPC> ActiveNPC;

public:
	
	//~ Begin AGimmickBase Interface
	virtual void Activate() override;
	//~ End AGimmickBase Interface
	
	//~ Begin AActor Interface
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface
	
};
