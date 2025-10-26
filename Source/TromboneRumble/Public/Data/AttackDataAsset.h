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
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> AttackAnimMontage = nullptr;

	UPROPERTY(EditAnywhere)
	float AttackCooldown = 1.0f;
	
	UPROPERTY(EditAnywhere)
	EHitType HitType = EHitType::Invalid;
};
