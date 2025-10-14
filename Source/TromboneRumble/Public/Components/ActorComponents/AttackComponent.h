// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

UCLASS(Abstract)
class TROMBONERUMBLE_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttackComponent();
	virtual void Attack() PURE_VIRTUAL(UAttackComponent::Attack, );
	virtual void SetOwner(ACharacter* InOwner);

protected:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;
};
