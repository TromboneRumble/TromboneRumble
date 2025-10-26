// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/AttackComponent.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAttackComponent::SetOwnerCharacter(ACharacter* InOwner)
{
	OwnerCharacter = InOwner;
}