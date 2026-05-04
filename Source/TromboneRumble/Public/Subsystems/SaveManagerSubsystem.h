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
	
	/** Load settings from slot, if not exist, create new settings with default values */
	UTromboneSaveGame* LoadOrCreateSettings();
	
	/** Apply all settings (audio, gameplay, video ...) */
	void ApplyAllSettings();
	
	/** Reset all settings to default values and save */
	void ResetToDefaultSettings();
	
public:
	
	/** Apply and save audio settings */
	void ApplyAudio(const FAudioSettingData& InAudioData, bool bSaveData = true);
	
	/** Apply and save gameplay settings */
	void ApplyGameplay(const FGameplaySettingData& InGameplayData, bool bSaveData = true);
	
	/** Apply video settings (save is optional) */
	void ApplyVideo(const FGraphicsSettingData& InVideoData, bool bSaveData = true);
	
public:
	
	/** if player is first time player, show tutorial popup */
	bool ShouldShowTutorialPopup() const;
	
	/** Mark tutorial as completed, will set bIsFirstTimePlayer to false and save */
	void MarkTutorialAsCompleted();
	
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
	
	// ~ Begin UGameInstanceSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// ~ End UGameInstanceSubsystem Interface
	
	void DumpTromboneSettings() const;
	
};