// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/ItemEquipHandler.h"
#include "InGameMode.generated.h"

UCLASS()
class TROMBONERUMBLE_API AInGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInGameMode();
	
	virtual void BeginPlay() override;
	
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
