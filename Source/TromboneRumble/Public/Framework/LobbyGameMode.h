// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Utilities/Defines.h"
#include "LobbyGameMode.generated.h"

class ADefaultTromboneCharacter;
class ADefaultPlayerState;
class ALobbyGameState;

UCLASS()
class TROMBONERUMBLE_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALobbyGameMode();
	virtual void BeginPlay() override;
	virtual void Logout(AController* ExitedPlayer) override;
	
	void NotifyClientReady(APlayerController* ReadyPlayer);
	void RequestServerTravel(EGameState InGameState);
	void SubscribeCharacterEvents(ADefaultTromboneCharacter* Character) const;
	
private:
	void InitializeMapPath();
	void InitializeInstruments() const;
	bool CheckAllClientsReady();
	void SetLobbyState(ELobbyState NewState);
	void RequestServerTravel(const FString& MapPath) const;
	void RequestSetTimer(TFunction<void()> OnTimerFinished);

	// Delegate Handlers
	UFUNCTION()
	void HandleInstrumentEquipped(APawn* EquippedPlayer, AInstrumentBase* EquippedInstrument);
	UFUNCTION()
	void HandleInstrumentUnequipped(APawn* UnequippedPlayer, AInstrumentBase* UnequippedInstrument);
	// ~Delegate Handlers

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