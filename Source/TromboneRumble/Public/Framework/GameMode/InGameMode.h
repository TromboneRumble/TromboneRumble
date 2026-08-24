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

	/** @return How many players the end sequence should wait for. Never returns zero. */
	int32 GetExpectedPlayerCount() const;

	// 결과 데이터 저장 + End 브로드캐스트 + 클라 이동 요청까지 한 묶음
	void StartEndSequence(int32 InSessionPlayerCount);

	// 클라 보고와 이탈 양쪽에서 호출된다. 이동 조건이 찼는지만 다시 본다
	void TryTravelToResultLevel();

	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> InGameReadyPlayers;

	// 리듬게임 종료를 보고한 플레이어 (Ended 브로드캐스트 경로가 둘이라 중복 보고 무시용)
	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> RhythmEndedPlayers;

	bool bInGamePlayStarted = false;

	// 종료 보고 경로가 둘이라 중복으로 들어온다. 종료 시퀀스는 한 번만 돌아야 한다
	bool bEndSequenceStarted = false;

	// 이동을 예약한 뒤에는 이탈이 더 들어와도 다시 예약하지 않는다
	bool bResultTravelStarted = false;

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
