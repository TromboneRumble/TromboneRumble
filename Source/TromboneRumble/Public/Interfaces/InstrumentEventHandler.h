// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InstrumentEventHandler.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInstrumentEventHandler : public UInterface
{
	GENERATED_BODY()
};

class TROMBONERUMBLE_API IInstrumentEventHandler
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void NotifyInstrumentEquipped(APlayerController* EquippedPlayer, AActor* EquippedInstrument);
	
	UFUNCTION()
	virtual void NotifyInstrumentUnequipped(APlayerController* UnequippedPlayer, AActor* UnequippedInstrument);
};
