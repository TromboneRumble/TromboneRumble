// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/InstrumentEquipHandler.h"
#include "InGameMode.generated.h"

UCLASS()
class TROMBONERUMBLE_API AInGameMode : public AGameModeBase, public IInstrumentEquipHandler
{
	GENERATED_BODY()

public:
	AInGameMode();
	
	// IInstrumentEquipHandler Interfaces
	virtual void HandleInstrumentEquipped(APawn* EquippedPlayer, AInstrumentBase* EquippedInstrument) override;
	virtual void HandleInstrumentUnequipped(APawn* UnequippedPlayer, AInstrumentBase* UnequippedInstrument) override;
	// ~IInstrumentEquipHandler Interfaces
};
