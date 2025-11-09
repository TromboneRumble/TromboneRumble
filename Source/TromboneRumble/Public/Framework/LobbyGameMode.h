// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/ItemEquipHandler.h"
#include "Utilities/Defines.h"
#include "LobbyGameMode.generated.h"

class AInstrumentBase;
class ADefaultTromboneCharacter;
class ADefaultPlayerState;
class ALobbyGameState;

UCLASS()
class TROMBONERUMBLE_API ALobbyGameMode : public AGameModeBase, public IItemEquipHandler
{
	GENERATED_BODY()
	
public:
	ALobbyGameMode();

	// IInstrumentEquipHandler Interfaces
	virtual void HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem) override;
	virtual void HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem) override;
	// ~IInstrumentEquipHandler Interfaces
	
	virtual void BeginPlay() override;
	virtual void Logout(AController* ExitedPlayer) override;
	
	void NotifyClientReady(APlayerController* ReadyPlayer);
	void RequestServerTravel(EGameState InGameState);

private:
	void InitializeMapPath();
	void InitializeInstruments() const;
	bool CheckAllClientsReady();
	void SetLobbyState(ELobbyState NewState);
	void RequestServerTravel(const FString& MapPath) const;
	void RequestSetTimer(TFunction<void()> OnTimerFinished);

private:
	UPROPERTY(Transient)
	FString CachedInGameMapPath;
	
	UPROPERTY(Transient)
	FString CachedLobbyMapPath;
	
	UPROPERTY()
	TObjectPtr<ALobbyGameState> LobbyGameState;

	UPROPERTY(EditDefaultsOnly, Category = "Instrument")
	TSubclassOf<AActor> InstrumentToSpawn;
	
	FTimerHandle LobbyTimerHandle;
	int32 NumPublicConnections;
	int32 CurrentEquippedInstruments;
	float Timer;
};