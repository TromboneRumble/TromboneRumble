// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "InGameState.generated.h"

UCLASS()
class TROMBONERUMBLE_API AInGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	float GetSongProgress() const { return SongProgress; }

private:
	float SongProgress = 0.15f;
};
