// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/InstrumentEventHandler.h"
#include "Utilities/Defines.h"
#include "LobbyGameMode.generated.h"

class ADefaultPlayerState;
class ALobbyGameState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClientReadySignature, APlayerController*, ReadyPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInstrumentEquippedSignature, APlayerController*, EqippedPlayer, AActor*, EquippedInstrument);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInstrumentUnequippedSignature, APlayerController*, UnequippedPlayer, AActor*, UnequippedInstrument);

UCLASS()
class TROMBONERUMBLE_API ALobbyGameMode : public AGameModeBase, public IInstrumentEventHandler
{
	GENERATED_BODY()
	
public:
	ALobbyGameMode();
	virtual void BeginPlay() override;
	virtual void Logout(AController* ExitedPlayer) override;

	// IInstrumentEventHandler interface
	virtual void NotifyInstrumentEquipped(APlayerController* EquippedPlayer, AActor* EquippedInstrument) override { OnInstrumentEquippedDelegate.Broadcast(EquippedPlayer, EquippedInstrument); }
	virtual void NotifyInstrumentUnequipped(APlayerController* UnequippedPlayer, AActor* UnequippedInstrument) override { OnInstrumentUnequippedDelegate.Broadcast(UnequippedPlayer, UnequippedInstrument); }
	// ~ IInstrumentEventHandler interface
	
	void RequestServerTravel(EGameState InGameState);
	
	FORCEINLINE void OnClientReady(APlayerController* ReadyPlayer) const { OnClientReadyDelegate.Broadcast(ReadyPlayer); }
	
public:
	FOnClientReadySignature OnClientReadyDelegate;
	FOnInstrumentEquippedSignature OnInstrumentEquippedDelegate;
	FOnInstrumentUnequippedSignature OnInstrumentUnequippedDelegate;

private:
	void InitializeMapPath();
	void InitializeInstruments() const;
	bool CheckAllClientsReady();
	void SetLobbyState(ELobbyState NewState);
	void RequestServerTravel(const FString& MapPath) const;
	void RequestSetTimer(TFunction<void()> OnTimerFinished);

	UFUNCTION()
	void HandleClientReady(APlayerController* ReadyPlayer);
	UFUNCTION()
	void HandleInstrumentEquipped(APlayerController* EquippedPlayer, AActor* EquippedInstrument);

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