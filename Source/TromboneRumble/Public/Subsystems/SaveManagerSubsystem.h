// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveManagerSubsystem.generated.h"

struct FGraphicsSettingData;
struct FGameplaySettingData;
struct FAudioSettingData;
class UTromboneSaveGame;

UCLASS()
class TROMBONERUMBLE_API USaveManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	void InitializeSettings();

	UTromboneSaveGame* GetCurrentCustomSettings();
    
	void SaveAudioSettings(const FAudioSettingData& NewSettings);
	void SaveGameplaySettings(const FGameplaySettingData& NewSettings);
	void SaveVideoSettings(const FGraphicsSettingData& NewSettings);

	void ApplyAudio(const FAudioSettingData& Settings);
	void ApplyGameplay(const FGameplaySettingData& Settings);

private:
	const FString SlotName = TEXT("TromboneSettings");
	const int32 UserIndex = 0;

	void InternalSave(UTromboneSaveGame* SaveObj);
};