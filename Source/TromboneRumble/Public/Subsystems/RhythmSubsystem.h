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
	
	FOnRhythmGameEndedDelegate OnRhythmGameEnded;

private:
	UFUNCTION()
	void HandleMusicCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	
	void OnMusicAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	void BroadcastUserCue(const FName& CueName);
};
