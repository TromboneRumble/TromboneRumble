#pragma once

#include "CoreMinimal.h"
#include "TromboneGameModeBase.h"
#include "Interfaces/ItemEquipHandler.h"
#include "LobbyGameMode.generated.h"

class ULobbyDirectorComponent;
class UPlayerReadyCheckComponent;

/** 
 * 로비 연출(낙하 → 기상 → 악기 쟁탈)은 LobbyDirectorComponent
 * 전원 로딩 완료 판정은 PlayerReadyCheckComponent
 */
UCLASS()
class TROMBONERUMBLE_API ALobbyGameMode : public ATromboneGameModeBase, public IItemEquipHandler
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	ALobbyGameMode();

	// IInstrumentEquipHandler Interfaces
	virtual void HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem) override;
	virtual void HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem) override;
	// ~IInstrumentEquipHandler Interfaces

private:

	void TravelToInGame();

private:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<ULobbyDirectorComponent> LobbyDirector;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPlayerReadyCheckComponent> PlayerReadyCheck;

public:

	// ~ Begin AGameModeBase Interface
	virtual void Logout(AController* ExitedPlayer) override;
	// ~ End AGameModeBase Interface

protected:

	// ~ Begin AGameModeBase Interface
	virtual void BeginPlay() override;
	// ~ End AGameModeBase Interface
};
