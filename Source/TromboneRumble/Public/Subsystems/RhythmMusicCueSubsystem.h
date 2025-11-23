// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AkGameplayTypes.h"
#include "RhythmMusicCueSubsystem.generated.h"

enum class EInstrumentType : uint8;
class UAkCallbackInfo;
class UAkMusicSyncCallbackInfo;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicUserCue, FName, CueName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnNotesUserCue, FName, CueName);


/**
 */
UCLASS()
class TROMBONERUMBLE_API URhythmMusicCueSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnMusicUserCue OnMusicUserCue;
	
	UFUNCTION()
	void OnMusicAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);

private:
	void BroadcastUserCue(const FName& CueName);
};
