// Fill out your copyright notice in the Description page of Project Settings.


#include "Interfaces/InstrumentEventHandler.h"

// Add default functionality here for any IInstrumentEventHandler functions that are not pure virtual.
void IInstrumentEventHandler::NotifyInstrumentEquipped(APlayerController* EquippedPlayer, AActor* EquippedInstrument)
{
}

void IInstrumentEventHandler::NotifyInstrumentUnequipped(APlayerController* UnequippedPlayer,
	AActor* UnequippedInstrument)
{
}
