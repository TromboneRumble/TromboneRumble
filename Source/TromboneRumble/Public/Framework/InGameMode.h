// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/TromboneGameModeBase.h"
#include "InGameMode.generated.h"

UCLASS()
class TROMBONERUMBLE_API AInGameMode : public ATromboneGameModeBase
{
	GENERATED_BODY()

public:
	
	virtual void BeginPlay() override;
	virtual void Logout(AController* ExitedPlayer) override;

	void GameEnd() const;
	void OnRhythmGameEndedReport();

	UFUNCTION()
	void HandlePlayerLoadingFinished(APlayerController* PC);

private:
	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> InGameReadyPlayers;
	int32 SessionPlayerNumber = 1;
	int32 RhythmGameEndedPlayerCount = 0;
};
