#include "Subsystems/SaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "AkGameplayStatics.h"
#include "SaveData/TromboneSaveGame.h"

void USaveManagerSubsystem::InitializeSettings()
{
    const UTromboneSaveGame* CustomSettings = GetCurrentCustomSettings();
    ApplyAudio(CustomSettings->Audio);
    ApplyGameplay(CustomSettings->Gameplay);

    if (!GEngine) return;
    
    if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
    {
        VideoSettings->LoadSettings();
        VideoSettings->ApplySettings(true);
    }
}

UTromboneSaveGame* USaveManagerSubsystem::GetCurrentCustomSettings()
{
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        return Cast<UTromboneSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    }
    return Cast<UTromboneSaveGame>(UGameplayStatics::CreateSaveGameObject(UTromboneSaveGame::StaticClass()));
}

void USaveManagerSubsystem::SaveAudioSettings(const FAudioSettingData& NewSettings)
{
    UTromboneSaveGame* SaveObj = GetCurrentCustomSettings();
    SaveObj->Audio = NewSettings;
    InternalSave(SaveObj);
    ApplyAudio(NewSettings);
}

void USaveManagerSubsystem::SaveGameplaySettings(const FGameplaySettingData& NewSettings)
{
    UTromboneSaveGame* SaveObj = GetCurrentCustomSettings();
    SaveObj->Gameplay = NewSettings;
    InternalSave(SaveObj);
    ApplyGameplay(NewSettings);
}

void USaveManagerSubsystem::SaveVideoSettings(const FGraphicsSettingData& NewSettings)
{
    if (!GEngine) return;
    
    if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
    {
        VideoSettings->SetOverallScalabilityLevel(NewSettings.OverallQuality);
        
        VideoSettings->SetViewDistanceQuality(NewSettings.ViewDistance);
        VideoSettings->SetAntiAliasingQuality(NewSettings.AntiAliasing);
        VideoSettings->SetPostProcessingQuality(NewSettings.PostProcess);
        VideoSettings->SetShadowQuality(NewSettings.Shadow);
        VideoSettings->SetGlobalIlluminationQuality(NewSettings.GlobalIllumination);
        VideoSettings->SetReflectionQuality(NewSettings.Reflections);
        VideoSettings->SetTextureQuality(NewSettings.Texture);
        VideoSettings->SetVisualEffectQuality(NewSettings.Effects);
        
        VideoSettings->SetScreenResolution(NewSettings.Resolution);
        VideoSettings->SetVSyncEnabled(NewSettings.bVSync);
        VideoSettings->SetFullscreenMode(NewSettings.WindowMode);
        
        VideoSettings->ApplySettings(true);
        VideoSettings->SaveSettings();
    }
}

void USaveManagerSubsystem::ApplyAudio(const FAudioSettingData& Settings)
{
    UAkGameplayStatics::SetRTPCValue(nullptr, Settings.MasterVolume * 100.f, 0, nullptr, FName(TEXT("RTPC_MasterVolume")));
    UAkGameplayStatics::SetRTPCValue(nullptr, Settings.MusicVolume * 100.f, 0, nullptr, FName(TEXT("RTPC_MusicVolume")));
    UAkGameplayStatics::SetRTPCValue(nullptr, Settings.SFXVolume * 100.f, 0, nullptr, FName(TEXT("RTPC_SFXVolume")));
}

void USaveManagerSubsystem::ApplyGameplay(const FGameplaySettingData& Settings)
{
    // TODO
}

void USaveManagerSubsystem::InternalSave(UTromboneSaveGame* SaveObj)
{
    UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}