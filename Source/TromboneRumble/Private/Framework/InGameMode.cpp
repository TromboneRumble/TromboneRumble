// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/InGameMode.h"

AInGameMode::AInGameMode()
{
	bUseSeamlessTravel = true;
}

void AInGameMode::HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem)
{
}

void AInGameMode::HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem)
{
}