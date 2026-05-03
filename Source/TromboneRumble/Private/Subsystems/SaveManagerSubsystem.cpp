#include "Subsystems/SaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Data/WwiseData.h"
#include "SaveData/TromboneSaveGame.h"
#include "Utilities/EnumHelper.h"

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

void USaveManagerSubsystem::DumpTromboneSettings() const
{
    if (!CachedSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SaveManager] Dump failed: CachedSettings is null."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("===================================================="));
    UE_LOG(LogTemp, Log, TEXT("          [Trombone Rumble] Current Settings        "));
    UE_LOG(LogTemp, Log, TEXT("===================================================="));

    // Audio
    const FAudioSettingData& Audio = CachedSettings->Audio;
    UE_LOG(LogTemp, Log, TEXT("[Audio] Master: %.2f | BGM: %.2f | Music: %.2f | SFX: %.2f"), 
        Audio.MasterVolume, Audio.BGMVolume, Audio.MusicVolume, Audio.SFXVolume);

    // Gameplay
    const FGameplaySettingData& Gameplay = CachedSettings->Gameplay;
    const FString VOIPString = EnumHelper::EnumToString(Gameplay.VOIPSetting);
    UE_LOG(LogTemp, Log, TEXT("[Gameplay] Show Username: %s | VOIP Setting: %s"), 
        Gameplay.bShouldShowUsernameInGame ? TEXT("True") : TEXT("False"), *VOIPString);

    // Player
    const FPlayerData& Player = CachedSettings->PlayerData;
    UE_LOG(LogTemp, Log, TEXT("[Player] Is First Time: %s"), 
        Player.bIsFirstTimePlayer ? TEXT("True") : TEXT("False"));

    // Video
    if (const UGameUserSettings* VideoSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
    {
        const FIntPoint Res = VideoSettings->GetScreenResolution();
        const int32 Quality = VideoSettings->GetOverallScalabilityLevel();
        UE_LOG(LogTemp, Log, TEXT("[Video] Resolution: %dx%d | Scalability: %d"), 
            Res.X, Res.Y, Quality);
    }

    UE_LOG(LogTemp, Log, TEXT("===================================================="));
}
