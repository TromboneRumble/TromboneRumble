// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "InGameState.generated.h"


class ADefaultPlayerState;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, APlayerState*, UpdatedPlayer);


UCLASS()
class TROMBONERUMBLE_API AInGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnScoreChanged OnScoreChanged;

	UFUNCTION()
	void HandleLocalScoreChanged(APlayerState* UpdatedPlayerState);

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
};
