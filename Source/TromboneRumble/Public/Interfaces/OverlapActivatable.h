// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OverlapActivatable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UOverlapActivatable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TROMBONERUMBLE_API IOverlapActivatable
{
	GENERATED_BODY()

public:
	// 플레이어가 트리거 볼륨에 들어온 순간
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Overlap")
	void OnOverlapActivate(AActor* InstigatorActor);
};
