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
	
	/** Load settings from slot, if not exist, create new settings with default values */
	UTromboneSaveGame* LoadOrCreateSettings();
	
	/** Apply all settings (audio, gameplay, video ...) */
	void ApplyAllSettings();
	
	/** Reset all settings to default values and save */
	void ResetToDefaultSettings();
	
	/** Apply and save audio settings */
	void ApplyAndSaveAudio(const FAudioSettingData& NewAudio);
	
	/** Apply and save gameplay settings */
	void ApplyAndSaveGameplay(const FGameplaySettingData& NewGameplay);
	
	/** Apply and save video settings */
	void ApplyAndSaveVideo(const FGraphicsSettingData& NewVideo);

	/** Apply audio settings without saving */
	void ApplyAudio(const FAudioSettingData& Settings);
	
	/** Apply gameplay settings without saving */
	void ApplyGameplay(const FGameplaySettingData& Settings);
	
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
};