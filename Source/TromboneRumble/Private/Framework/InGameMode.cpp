// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/InGameMode.h"

AInGameMode::AInGameMode()
{
	bUseSeamlessTravel = true;
}

void AInGameMode::HandleInstrumentEquipped(APawn* EquippedPlayer, AInstrumentBase* EquippedInstrument)
{
}

void AInGameMode::HandleInstrumentUnequipped(APawn* UnequippedPlayer, AInstrumentBase* UnequippedInstrument)
{
}
