// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemBase.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Melee,
	Ranged,
	End UMETA(Hidden)
};

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class TROMBONERUMBLE_API AWeapon : public AItemBase
{
	GENERATED_BODY()

protected:
};
