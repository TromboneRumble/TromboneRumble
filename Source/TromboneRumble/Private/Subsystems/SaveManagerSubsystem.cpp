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
        VideoSettings->ApplySettings(false);
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

void USaveManagerSubsystem::SaveVideoSettings(int32 QualityLevel, EWindowMode::Type WindowMode, FIntPoint Resolution)
{
    if (!GEngine) return;
    
    if (UGameUserSettings* VideoSettings = GEngine->GetGameUserSettings())
    {
        VideoSettings->SetOverallScalabilityLevel(QualityLevel);
        VideoSettings->SetFullscreenMode(WindowMode);
        VideoSettings->SetScreenResolution(Resolution);
        
        VideoSettings->ApplySettings(false);
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