// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/ItemEquipHandler.h"
#include "TutorialGameMode.generated.h"

UCLASS()
class TROMBONERUMBLE_API ATutorialGameMode : public AGameModeBase, public IItemEquipHandler
{
	GENERATED_BODY()
	
public:
	ATutorialGameMode();

public:
	// IInstrumentEquipHandler Interfaces
	virtual void HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem) override;
	virtual void HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem) override;
	// ~IInstrumentEquipHandler Interfaces
	
};
