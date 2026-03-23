#pragma once

#include "CoreMinimal.h"
#include "TromboneGameModeBase.h"
#include "Interfaces/ItemEquipHandler.h"
#include "TutorialGameMode.generated.h"

UCLASS()
class TROMBONERUMBLE_API ATutorialGameMode : public ATromboneGameModeBase, public IItemEquipHandler
{
	GENERATED_BODY()
	
public:
	
	// IInstrumentEquipHandler Interfaces
	virtual void HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem) override;
	virtual void HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem) override;
	// ~IInstrumentEquipHandler Interfaces
	
};
