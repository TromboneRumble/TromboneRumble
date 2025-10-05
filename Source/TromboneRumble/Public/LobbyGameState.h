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
	UFUNCTION()
	void OnRep_SessionPlayerList() const;
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_SessionPlayerList)
	TArray<FString> PlayerList;
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
