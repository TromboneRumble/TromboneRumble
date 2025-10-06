// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

UCLASS()
class TROMBONERUMBLE_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdatePlayerList();
	
private:
	UFUNCTION()
	void OnRep_SessionPlayerList() const;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_SessionPlayerList)
	TArray<FString> PlayerList;
};
