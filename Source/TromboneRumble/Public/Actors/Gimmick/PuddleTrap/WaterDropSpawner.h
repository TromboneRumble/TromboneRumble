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

	// 물방울 스폰 간격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	float SpawnInterval = 10.f;

	FTimerHandle SpawnTimerHandle;

	void SpawnOneDrop();

};
