// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Equipable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEquipable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TROMBONERUMBLE_API IEquipable
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equip")
	void Equip(AActor* OwnerActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equip")
	void Unequip(AActor* OwnerActor);
};
