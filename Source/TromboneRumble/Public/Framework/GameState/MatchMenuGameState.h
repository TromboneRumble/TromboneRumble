// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MatchMenuGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerListUpdateSignature, const TArray<FString>&, PlayerNames);

UCLASS()
class TROMBONERUMBLE_API AMatchMenuGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdatePlayerList();
	
	FOnPlayerListUpdateSignature OnPlayerListChanged;
	
	void SetIsTransitioningToInGame(const bool bInIsTransitioning);
	
private:
	UFUNCTION()
	void OnRep_PlayerList() const;
	
	UPROPERTY(ReplicatedUsing = OnRep_PlayerList)
	TArray<FString> PlayerList;
	
	UFUNCTION()
	void OnRep_IsTransitioningToInGame();
	
	UPROPERTY(ReplicatedUsing = OnRep_IsTransitioningToInGame)
	bool bIsTransitioningToInGame = false;

public:
	// ~ Begin Getter & Setter
	FORCEINLINE TArray<FString> GetPlayerList() const { return PlayerList; }
	// ~ End Getter & Setter
};