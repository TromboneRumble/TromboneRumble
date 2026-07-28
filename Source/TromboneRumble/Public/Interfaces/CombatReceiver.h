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

	/** 넉백 방향. 수신 측에서 수평 성분만 사용한다 (수직은 KnockbackUpForce 로 별도 지정) */
	UPROPERTY()
	FVector HitDirection = FVector::ZeroVector;

	/** 수평 넉백 힘 */
	UPROPERTY()
	float KnockbackForce = 0.0f;

	/** 수직(상향) 넉백 힘 */
	UPROPERTY()
	float KnockbackUpForce = 0.0f;

	UPROPERTY()
	EHitInstigatorType HitInstigator = EHitInstigatorType::None;

	UPROPERTY()
	EHitReactionType HitReaction = EHitReactionType::None;

	UPROPERTY()
	FVector ImpactPoint = FVector::ZeroVector;

	UPROPERTY()
	float ExplosionRadius = 0.0f;

	UPROPERTY()
	float ExplosionStrength = 0.0f;
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
