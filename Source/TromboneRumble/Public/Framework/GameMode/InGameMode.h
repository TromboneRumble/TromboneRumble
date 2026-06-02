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

private:
	
	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> InGameReadyPlayers;
	
	int32 SessionPlayerNumber = 1;
	
	int32 RhythmGameEndedPlayerCount = 0;
	
	int32 ClientsTravelToResultSceneCount = 0;
	
	FTimerHandle TimerHandle_TravelToResultLevel;
	
public:
	
	// ~ Begin AGameModeBase Interface
	virtual void Logout(AController* ExitedPlayer) override;
	// ~ End AGameModeBase Interface
	
protected:
	
	// ~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End AActor Interface
};
