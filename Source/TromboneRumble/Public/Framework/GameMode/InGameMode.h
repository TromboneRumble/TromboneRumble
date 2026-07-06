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

	void OnRhythmGameEndedReport(APlayerController* PC);

	void OnClientTravelToResultLevelAndLeaveSession();

	UFUNCTION()
	void HandlePlayerLoadingFinished(APlayerController* PC);

private:
	
	void TryStartInGamePlay();
	
	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> InGameReadyPlayers;

	// 리듬게임 종료를 보고한 플레이어 (Ended 브로드캐스트 경로가 둘이라 중복 보고 무시용)
	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> RhythmEndedPlayers;

	bool bInGamePlayStarted = false;

	int32 ClientsTravelToResultSceneCount = 0;

	FTimerHandle TimerHandle_TravelToResultLevel;

	FTimerHandle TimerHandle_RetryStartInGame;
	
public:
	
	// ~ Begin AGameModeBase Interface
	virtual void Logout(AController* ExitedPlayer) override;
	// ~ End AGameModeBase Interface
	
protected:
	
	// ~ Begin AActor Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End AActor Interface
};
