#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TromboneGameModeBase.h"
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
	
	/** Default constructor. */
	ALobbyGameMode();
	
	// IInstrumentEquipHandler Interfaces
	virtual void HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem) override;
	virtual void HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem) override;
	// ~IInstrumentEquipHandler Interfaces

private:
	void HandlePlayerLoadingScreenFinished(APlayerController* PC);

	/** Resolves the currently selected InGame map tag (SETTING_MAPNAME, falls back to DefaultInGameMap). */
	FGameplayTag ResolveSelectedInGameMapTag() const;
	/** Randomly picks a rhythm song tag matching the given InGame map (server-authoritative). */
	FGameplayTag PickRandomSongForMap(const FGameplayTag& InGameMapTag) const;

	void SpawnInstruments();
	void SetLobbyState(const ELobbyState& InNewState);
	void RequestServerTravel(const FString& MapPath) const;
	
	/** Called when all player equipped with instrument */
	void OnCountdownToTravel();

private:
	
	UPROPERTY()
	TObjectPtr<ALobbyGameState> LobbyGameState;

	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> LobbyReadyPlayers;
	
	FTimerHandle LobbyTimerHandle;

	/** Number of players registered for the session */
	int32 RegisteredPlayerCount;
	
	/** Number of Instruments spawned in the lobby. */
	int32 SpawnedInstrumentCount;
	
	/** Number of Instruments equipped by players in the lobby. */
	int32 EquippedInstrumentCount;
	
	/** Delay time before start */
	float DelayTime;
	
public:
	
	// ~ Begin AGameModeBase Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* ExitedPlayer) override;
	// ~ End AGameModeBase Interface
};