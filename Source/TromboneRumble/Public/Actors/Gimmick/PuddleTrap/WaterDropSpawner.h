// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterDropSpawner.generated.h"

class UBoxComponent;
class AWaterDrop;

UCLASS(Abstract)
class TROMBONERUMBLE_API AWaterDropSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaterDropSpawner();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> SpawnBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AWaterDrop> WaterDropClass;

	/** 물방울이 생성되는 시간 간격 (초) 
	 * 값이 작을수록 물방울이 더 자주 생성되어 떨어집니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaterDrop|Config", meta = (DisplayName = "물방울 생성 간격"))
	float SpawnInterval = 10.f;

	FTimerHandle SpawnTimerHandle;

	void SpawnOneDrop();

};
