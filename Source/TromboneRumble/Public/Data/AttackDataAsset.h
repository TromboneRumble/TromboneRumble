// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utilities/Defines.h"
#include "AttackDataAsset.generated.h"

UCLASS()
class TROMBONERUMBLE_API UAttackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	TObjectPtr<UAnimMontage> AttackAnimMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	float AttackCooldown = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	EHitType HitType = EHitType::Invalid;
};
