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
	
	virtual bool CanAttack() const = 0;
	
	/** @param Duration 공격 몽타주의 길이 */
	virtual void BeginAttack(float Duration) = 0;
	
	virtual void EndAttack() = 0;
	
};
