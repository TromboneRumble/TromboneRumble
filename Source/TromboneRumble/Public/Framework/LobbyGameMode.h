#pragma once

#include "CoreMinimal.h"
#include "GameMode/TromboneGameModeBase.h"
#include "Interfaces/ItemEquipHandler.h"
#include "Utilities/Defines.h"
#include "LobbyGameMode.generated.h"

class ADefaultTromboneCharacter;
class ADefaultPlayerState;
class ALobbyGameState;

UCLASS()
class TROMBONERUMBLE_API ALobbyGameMode : public ATromboneGameModeBase, public IItemEquipHandler
{
	GENERATED_BODY()
	
public:
	// IInstrumentEquipHandler Interfaces
	virtual void HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem) override;
	virtual void HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem) override;
	// ~IInstrumentEquipHandler Interfaces
	
	virtual void BeginPlay() override;
	virtual void Logout(AController* ExitedPlayer) override;

	void RequestServerTravel(const ELevelState& InLevelState);

private:
	void HandlePlayerLoadingScreenFinished(APlayerController* PC);
	void InitializeInstruments() const;
	void SetLobbyState(const ELobbyState& InNewState);
	void RequestServerTravel(const FString& MapPath) const;
	void RequestSetTimer(TFunction<void()> OnTimerFinished);
	

private:	
	UPROPERTY()
	TObjectPtr<ALobbyGameState> LobbyGameState;

	UPROPERTY(Transient)
	FTimerHandle LobbyTimerHandle;

	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> LobbyReadyPlayers;

	int32 RegisteredPlayerCount = 4;
	int32 CurrentEquippedInstruments = 0;
	float Timer = 5.0f;
};