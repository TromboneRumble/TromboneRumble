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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstrumentPickedDelegate, EInstrumentType, InType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoteDetectedDelegate, ENoteResult, InNoteResult);

/**
 */
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

private:
	UFUNCTION()
	void OnMusicAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	void BroadcastUserCue(const FName& CueName);
};
