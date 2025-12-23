// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Weapon.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWeapon : public UInterface
{
	GENERATED_BODY()
};

class TROMBONERUMBLE_API IWeapon
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool IsCanAttack() const = 0;
	virtual void BeginAttack() = 0;
	virtual void EndAttack() = 0;
};
