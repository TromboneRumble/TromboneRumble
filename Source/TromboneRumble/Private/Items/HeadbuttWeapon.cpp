// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/HeadbuttWeapon.h"

void AHeadbuttWeapon::OnHitSuccess(AActor* HitActor)
{
	Super::OnHitSuccess(HitActor);
	
	if (!IsOwnerLocallyControlled()) return;

	PlayHitSound();
}
