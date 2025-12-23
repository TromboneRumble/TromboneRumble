// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utilities/Defines.h"
#include "WeaponDataAsset.generated.h"

UCLASS()
class TROMBONERUMBLE_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	float AttackCooldown = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	float KnockbackForce = 500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	float WeaponDropForwardImpulse = 500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	float WeaponDropUpwardImpulse = 300.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	EHitType HitType = EHitType::Invalid;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	EWeaponType WeaponType = EWeaponType::Invalid;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Attack")
	FName EquipSocketName = TEXT("socket_hand_l");
};
