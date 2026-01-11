// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Utilities/Defines.h"
#include "CombatReceiver.generated.h"

USTRUCT(BlueprintType)
struct FHitData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector HitDirection = FVector::ZeroVector;
	
	UPROPERTY()
	float KnockbackForce = 0.0f;

	UPROPERTY()
	EHitReactionType HitType = EHitReactionType::Invalid;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatReceiver : public UInterface
{
	GENERATED_BODY()
};

class TROMBONERUMBLE_API ICombatReceiver
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void OnHitReceived(const FHitData& HitData);
};
