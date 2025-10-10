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

	void UpdatePlayerList();
	void SetLobbyState(ELobbyState NewState);
	
	FORCEINLINE TArray<FString> GetPlayerList() const { return PlayerList; }
	FORCEINLINE ELobbyState GetCurrentLobbyState() const { return CurrentLobbyState; }
	FORCEINLINE ELobbyState GetPreviousLobbyState() const { return PreviousLobbyState; }
	FORCEINLINE bool IsInState(const ELobbyState State) const { return CurrentLobbyState == State; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveWall();

public:
	FOnLobbyStateChangedSignature OnLobbyStateChanged;
	
private:
	UFUNCTION()
	void OnRep_SessionPlayerList() const;

	UFUNCTION()
	void OnRep_LobbyState() const;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_SessionPlayerList)
	TArray<FString> PlayerList;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	ELobbyState CurrentLobbyState;
	ELobbyState PreviousLobbyState;
};
