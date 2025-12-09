// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterDrop.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class APuddleTrap;

/// <summary>
/// 땅에 부딪히면 PuddleTrap 생성
/// </summary>
UCLASS(Abstract)
class TROMBONERUMBLE_API AWaterDrop : public AActor
{
	GENERATED_BODY()
	
public:
	AWaterDrop();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Puddle")
	TSubclassOf<APuddleTrap> PuddleTrapClass;

	bool bHasSpawnedPuddle = false;

};
