// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

enum class ELobbyState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyStateChangedSignature, ELobbyState, NewState);

UCLASS()
class TROMBONERUMBLE_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	TArray<FString> GetPlayerList() const { return PlayerList; }
	void UpdatePlayerList();
	
	FORCEINLINE ELobbyState GetLobbyState() const { return CurrentLobbyState; }
	void SetLobbyState(ELobbyState NewState);

public:
	FOnLobbyStateChangedSignature OnLobbyStateChanged;
	
private:
	UFUNCTION()
	void OnRep_SessionPlayerList() const;

	UFUNCTION()
	void OnRep_LobbyState();
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_SessionPlayerList)
	TArray<FString> PlayerList;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	ELobbyState CurrentLobbyState;
};
