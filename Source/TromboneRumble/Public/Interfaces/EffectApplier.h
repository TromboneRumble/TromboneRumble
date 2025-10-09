// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EffectApplier.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEffectApplier : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TROMBONERUMBLE_API IEffectApplier
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Effect")
	void ApplyTo(AActor* Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Effect")
	void RemoveFrom(AActor* Target);
};
