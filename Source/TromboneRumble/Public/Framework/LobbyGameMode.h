// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

class ALobbyPlayerState;
class ALobbyGameState;
enum class ELobbyState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClientReadySignature, APlayerController*, ReadyPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstrumentEquippedSignature, APlayerController*, EqippedPlayer);

UCLASS()
class TROMBONERUMBLE_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALobbyGameMode();
	virtual void BeginPlay() override;
	virtual void Logout(AController* ExitedPlayer) override;

	FORCEINLINE void OnClientReady(APlayerController* ReadyPlayer) const { OnClientReadyDelegate.Broadcast(ReadyPlayer); }
	FORCEINLINE void OnInstrumentEquipped(APlayerController* EquippedPlayerState) const { OnInstrumentEquippedDelegate.Broadcast(EquippedPlayerState); }
	
public:
	FOnClientReadySignature OnClientReadyDelegate;
	FOnInstrumentEquippedSignature OnInstrumentEquippedDelegate;

private:
	void InitializeMapPath();
	bool CheckAllClientsReady();
	void SetLobbyState(ELobbyState NewState);
	void RequestServerTravel(const FString& MapPath) const;
	void RequestSetTimer(TFunction<void()> OnTimerFinished);

	UFUNCTION()
	void HandleClientReady(APlayerController* ReadyPlayer);
	UFUNCTION()
	void HandleInstrumentEquipped(APlayerController* EquippedPlayerState);

private:
	UPROPERTY(Transient)
	FString CachedInGameMapPath;
	
	UPROPERTY(Transient)
	FString CachedLobbyMapPath;
	
	UPROPERTY()
	TObjectPtr<ALobbyGameState> LobbyGameState;

	FTimerHandle LobbyTimerHandle;
	int32 MaxPlayers;
	int32 CurrentEquippedInstruments;
	float Timer;
};