// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

class ALobbyGameState;
enum class ELobbyState : uint8;

UCLASS()
class TROMBONERUMBLE_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALobbyGameMode();
	virtual void BeginPlay() override;
	virtual void Logout(AController* ExitedPlayer) override;
	void OnClientIsReady(const APlayerController* ReadyPlayer);

private:
	bool CheckAllClientsReady();
	void SetLobbyState(ELobbyState NewState);
	void ServerTravelToInGame() const;
	void StartTimer(TFunction<void()> OnTimerFinished);
	
private:
	UPROPERTY(Transient)
	FString CachedInGameMapPath = TEXT("");
	
	UPROPERTY()
	TObjectPtr<ALobbyGameState> LobbyGameState;
	
	FTimerHandle LobbyTimerHandle;
	int32 MaxPlayers = 2;
	float Timer = 5.0f; // TODO : delete magic number
};