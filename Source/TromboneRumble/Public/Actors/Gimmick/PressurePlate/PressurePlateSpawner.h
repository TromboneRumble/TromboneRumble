// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "PressurePlateSpawner.generated.h"

class ATargetPoint;
class APressurePlateBase;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API APressurePlateSpawner : public AGimmickBase
{
	GENERATED_BODY()
public:
	APressurePlateSpawner();

	virtual void Activate() override;
	virtual void Deactivate() override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnMusicCueReceived(FName CueName);

	UFUNCTION()
	void OnPlateDestroyed(AActor* DestroyedActor);

	void TriggerPlateSpawn();

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<APressurePlateBase> PressurePlateClass;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

	//압력판이 사라진 후 다시 나올때까지 걸리는 시간
	UPROPERTY(EditAnywhere, Category = "Config")
	float RespawnDelay_Normal = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 MaxPlates_Normal = 1;

	//피버타임때 압력판이 사라진 후 다시 나타날 때까지의 시간
	UPROPERTY(EditAnywhere, Category = "Config")
	float RespawnDelay_Fever = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 MaxPlates_Fever = 2;

	UPROPERTY()
	TMap<TObjectPtr<ATargetPoint>, TObjectPtr<APressurePlateBase>> SpawnMap;

	FTimerHandle SpawnTimerHandle;
	bool bIsFeverTime = false;
};
