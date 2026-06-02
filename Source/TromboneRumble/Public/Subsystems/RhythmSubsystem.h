// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AkGameplayTypes.h"
#include "GameplayTagContainer.h"
#include "Utilities/Defines.h"
#include "RhythmSubsystem.generated.h"

class ARhythmActor;
enum class EInstrumentType : uint8;
class UAkCallbackInfo;
class UAkMusicSyncCallbackInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMusicCallbackDelegate, EAkCallbackType, CallbackType, UAkCallbackInfo*, CallbackInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInstrumentPickedDelegate, EInstrumentType, PrevType, EInstrumentType, NewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoteDetectedDelegate, ENoteResult, InNoteResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRhythmGameStateDelegate, ERhythmGameState, CurrentGameState);

UCLASS()
class TROMBONERUMBLE_API URhythmSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void StartRhythmGame(const FGameplayTag& InGamePlayTag);

	UFUNCTION(BlueprintCallable)
	void PauseRhythmGame();
	UFUNCTION(BlueprintCallable)
	void ResumeRhythmGame();

	UFUNCTION(BlueprintCallable)
	void StopRhythmGame();

	UFUNCTION(BlueprintCallable)
	void EndRhythmGame();

	UFUNCTION()
	void HandleMusicCallbacksFromRhythmActor(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);

	UPROPERTY(BlueprintAssignable)
	FOnMusicCallbackDelegate OnMusicCallback;

	UPROPERTY(BlueprintAssignable)
	FOnInstrumentPickedDelegate OnInstrumentPicked;

	UPROPERTY(BlueprintAssignable)
	FOnNoteDetectedDelegate OnNoteDetected;

	//BGM의 PlayingID를 세팅해야해서 노트 소환이 아니라, 음악 재생 시점에서 게임 시작했다고 알림
	UPROPERTY(BlueprintAssignable)
	FOnRhythmGameStateDelegate OnRhythmGameStateChanged;
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UPROPERTY()
	TWeakObjectPtr<ARhythmActor> RhythmActor;

	ERhythmGameState CurrentState = ERhythmGameState::None;
private:
	void BroadcastUserCue(const FName& CueName);

	bool isRhythmGameForceStopped = false;

public:
	FORCEINLINE ERhythmGameState GetCurrentRhythmState() const { return CurrentState; }
	void RegisterRhythmActor(ARhythmActor* InActor);
};
