// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/HeadbuttWeapon.h"

void AHeadbuttWeapon::Multicast_OnHitSuccess_Implementation(AActor* HitActor)
{
	Super::Multicast_OnHitSuccess_Implementation(HitActor);
	PlayHitSound();
}