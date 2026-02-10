// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AkGameplayTypes.h"
#include "Utilities/Defines.h"
#include "RhythmSubsystem.generated.h"

enum class EInstrumentType : uint8;
class UAkCallbackInfo;
class UAkMusicSyncCallbackInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicUserCue, FName, CueName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInstrumentPickedDelegate, EInstrumentType, PrevType, EInstrumentType, NewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoteDetectedDelegate, ENoteResult, InNoteResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRhythmGameStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRhythmGameEndedDelegate);

UCLASS()
class TROMBONERUMBLE_API URhythmSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnMusicUserCue OnMusicUserCue;

	UPROPERTY(BlueprintAssignable)
	FOnInstrumentPickedDelegate OnInstrumentPicked;

	UPROPERTY(BlueprintAssignable)
	FOnNoteDetectedDelegate OnNoteDetected;

	//BGM의 PlayingID를 세팅해야해서 노트 소환이 아니라, 음악 재생 시점에서 게임 시작했다고 알림
	FOnRhythmGameStartedDelegate OnRhythmGameStarted;
	FOnRhythmGameEndedDelegate OnRhythmGameEnded;

	UFUNCTION()
	void HandleMusicCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
private:
	void OnMusicAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	void BroadcastUserCue(const FName& CueName);
};
