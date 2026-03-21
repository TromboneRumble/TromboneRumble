#include "Subsystems/SaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Data/WwiseData.h"
#include "SaveData/TromboneSaveGame.h"

void USaveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CachedSettings = LoadOrCreateSettings();
    ApplyAllSettings();
}

bool USaveManagerSubsystem::ShouldShowTutorialPopup() const
{
    return CachedSettings->PlayerData.bIsFirstTimePlayer;
}

void USaveManagerSubsystem::MarkTutorialAsCompleted()
{
    CachedSettings->PlayerData.bIsFirstTimePlayer = false;
    InternalSave();
}

void USaveManagerSubsystem::ResetToDefaultSettings()
{
    UTromboneSaveGame* DefaultSettings = Cast<UTromboneSaveGame>(UGameplayStatics::CreateSaveGameObject(UTromboneSaveGame::StaticClass()));

    if (DefaultSettings)
    {
        CachedSettings = DefaultSettings;
        InternalSave();

        if (GEngine)
        {
            if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
            {
                VideoSettings->SetToDefaults();
                VideoSettings->ApplySettings(false);
                VideoSettings->SaveSettings();
            }
        }

        ApplyAllSettings();
        
        UE_LOG(LogTemp, Log, TEXT("[USaveManagerSubsystem::ResetToDefaultSettings] All settings have been reset to default."));
    }
}

void USaveManagerSubsystem::ApplyAllSettings()
{
    ApplyAudio(CachedSettings->Audio);
    ApplyGameplay(CachedSettings->Gameplay);

    if (!GEngine) return;
    
    if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
    {
        VideoSettings->LoadSettings();
        VideoSettings->ApplySettings(true);
    }
}

UTromboneSaveGame* USaveManagerSubsystem::LoadOrCreateSettings()
{
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        return Cast<UTromboneSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    }
    return Cast<UTromboneSaveGame>(UGameplayStatics::CreateSaveGameObject(UTromboneSaveGame::StaticClass()));
}

void USaveManagerSubsystem::ApplyAndSaveAudio(const FAudioSettingData& NewAudio)
{
    CachedSettings->Audio = NewAudio;
    InternalSave();
    ApplyAudio(NewAudio);
}

void USaveManagerSubsystem::ApplyAndSaveGameplay(const FGameplaySettingData& NewGameplay)
{
    CachedSettings->Gameplay = NewGameplay;
    InternalSave();
    ApplyGameplay(NewGameplay);
}

void USaveManagerSubsystem::ApplyAndSaveVideo(const FGraphicsSettingData& NewVideo)
{
    if (!GEngine) return;
    
    if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
    {
        VideoSettings->SetOverallScalabilityLevel(NewVideo.OverallQuality);
        
        VideoSettings->SetViewDistanceQuality(NewVideo.ViewDistance);
        VideoSettings->SetAntiAliasingQuality(NewVideo.AntiAliasing);
        VideoSettings->SetPostProcessingQuality(NewVideo.PostProcess);
        VideoSettings->SetShadowQuality(NewVideo.Shadow);
        VideoSettings->SetGlobalIlluminationQuality(NewVideo.GlobalIllumination);
        VideoSettings->SetReflectionQuality(NewVideo.Reflections);
        VideoSettings->SetTextureQuality(NewVideo.Texture);
        VideoSettings->SetVisualEffectQuality(NewVideo.Effects);
        
        VideoSettings->SetScreenResolution(NewVideo.Resolution);
        VideoSettings->SetVSyncEnabled(NewVideo.bVSync);
        VideoSettings->SetFullscreenMode(NewVideo.WindowMode);
        
        VideoSettings->ApplySettings(true);
        VideoSettings->SaveSettings();
    }
}

void USaveManagerSubsystem::ApplyAudio(const FAudioSettingData& Settings)
{
    WwiseRTPC::SetVolume(WwiseRTPC::MasterVolume, Settings.MasterVolume);
    WwiseRTPC::SetVolume(WwiseRTPC::BGMVolume, Settings.BGMVolume);
    WwiseRTPC::SetVolume(WwiseRTPC::MusicVolume, Settings.MusicVolume);
    WwiseRTPC::SetVolume(WwiseRTPC::SFXVolume, Settings.SFXVolume);
}

void USaveManagerSubsystem::ApplyGameplay(const FGameplaySettingData& Settings)
{
    // TODO
}

void USaveManagerSubsystem::InternalSave()
{
    UGameplayStatics::SaveGameToSlot(CachedSettings, SlotName, UserIndex);
}