#include "Subsystems/SaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Data/WwiseData.h"
#include "Kismet/KismetInternationalizationLibrary.h"
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
    ApplyAudio(CachedSettings->Audio, false);
    ApplyGameplay(CachedSettings->Gameplay, false);

    if (GEngine)
    {
        if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
        {
            VideoSettings->LoadSettings();
            VideoSettings->ApplySettings(false);
        }
    }
    
    if (GConfig)
    {
        const FString CurrentCulture = GConfig->GetStr(TEXT("Internationalization"), TEXT("Language"), GGameUserSettingsIni);
        UKismetInternationalizationLibrary::SetCurrentLanguage(CurrentCulture, false);
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

void USaveManagerSubsystem::ApplyAudio(const FAudioSettingData& InAudioData, bool bSaveData)
{
    WwiseRTPC::SetVolume(WwiseRTPC::MasterVolume, InAudioData.MasterVolume);
    WwiseRTPC::SetVolume(WwiseRTPC::BGMVolume, InAudioData.BGMVolume);
    WwiseRTPC::SetVolume(WwiseRTPC::MusicVolume, InAudioData.MusicVolume);
    WwiseRTPC::SetVolume(WwiseRTPC::SFXVolume, InAudioData.SFXVolume);
    
    if (bSaveData)
    {
        CachedSettings->Audio = InAudioData;
        InternalSave();
    }
}

void USaveManagerSubsystem::ApplyGameplay(const FGameplaySettingData& InGameplayData, bool bSaveData)
{
    // TODO : 게임 플레이 적용 구문
    
    if (bSaveData)
    {
        CachedSettings->Gameplay = InGameplayData;
        InternalSave();
    }
}

void USaveManagerSubsystem::ApplyVideo(const FGraphicsSettingData& InVideoData, bool bSaveData)
{
    if (!GEngine) return;
    
    if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
    {
        VideoSettings->SetOverallScalabilityLevel(InVideoData.OverallQuality);
        
        VideoSettings->SetViewDistanceQuality(InVideoData.ViewDistance);
        VideoSettings->SetAntiAliasingQuality(InVideoData.AntiAliasing);
        VideoSettings->SetPostProcessingQuality(InVideoData.PostProcess);
        VideoSettings->SetShadowQuality(InVideoData.Shadow);
        VideoSettings->SetGlobalIlluminationQuality(InVideoData.GlobalIllumination);
        VideoSettings->SetReflectionQuality(InVideoData.Reflections);
        VideoSettings->SetTextureQuality(InVideoData.Texture);
        VideoSettings->SetVisualEffectQuality(InVideoData.Effects);
        
        VideoSettings->SetScreenResolution(InVideoData.Resolution);
        VideoSettings->SetVSyncEnabled(InVideoData.bVSync);
        VideoSettings->SetFullscreenMode(InVideoData.WindowMode);
        
        VideoSettings->ApplySettings(false);
        
        if (bSaveData)
        {
            VideoSettings->SaveSettings();
        }
    }
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
