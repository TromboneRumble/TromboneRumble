// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/AttackComponent.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttackComponent::SetOwner(ACharacter* InOwner)
{
	OwnerCharacter = InOwner;
}