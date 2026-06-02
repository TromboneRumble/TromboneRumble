// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TromboneGameModeBase.h"
#include "InGameMode.generated.h"

UCLASS()
class TROMBONERUMBLE_API AInGameMode : public ATromboneGameModeBase
{
	GENERATED_BODY()

public:
	
	void OnRhythmGameEndedReport();
	
	void OnClientTravelToResultLevelAndLeaveSession();

	UFUNCTION()
	void HandlePlayerLoadingFinished(APlayerController* PC);
	
protected:
	
	virtual void BeginPlay() override;

private:
	
	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> InGameReadyPlayers;
	
	int32 SessionPlayerNumber = 1;
	
	int32 RhythmGameEndedPlayerCount = 0;
	
	int32 ClientsTravelToResultSceneCount = 0;
};
