// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveData/TromboneSaveGame.h"
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
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void ApplyAllSettings();
	UTromboneSaveGame* LoadOrCreateSettings();
    
	void UpdateAndSaveAudio(const FAudioSettingData& NewAudio);
	void UpdateAndSaveGameplay(const FGameplaySettingData& NewGameplay);
	void SaveVideo(const FGraphicsSettingData& NewVideo);

	void ApplyAudio(const FAudioSettingData& Settings);
	void ApplyGameplay(const FGameplaySettingData& Settings);
	
private:
	void InternalSave();
	
	UPROPERTY()
	TObjectPtr<UTromboneSaveGame> CachedSettings;
	
	const FString SlotName = TEXT("TromboneSettings");
	const int32 UserIndex = 0;
	
public:
	// ~ Begin Getter
	TObjectPtr<UTromboneSaveGame> GetSettings() const { return CachedSettings; }
	FAudioSettingData GetAudioSettings() const { return CachedSettings->Audio; }
	FGameplaySettingData GetGameplaySettings() const { return CachedSettings->Gameplay; }
	// ~ End Getter
};