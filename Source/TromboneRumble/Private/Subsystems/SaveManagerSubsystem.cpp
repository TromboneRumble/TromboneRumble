#include "Subsystems/SaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Data/WwiseData.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "SaveData/TromboneSaveGame.h"
#include "Utilities/EnumHelper.h"
#include "Subsystems/VoiceChatSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Misc/AES.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

void USaveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CachedSettings = LoadOrCreateSettings();
    ApplyAllSettings();
    CachedCustomization = LoadCustomizationFromDisk();
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
    
    const FString CurrentLanguage = UKismetInternationalizationLibrary::GetCurrentLanguage();
    UKismetInternationalizationLibrary::SetCurrentLanguage(CurrentLanguage, false);
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

    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULocalPlayer* LP = GI->GetFirstGamePlayer())
        {
            if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
            {
                VCS->ApplyMicrophoneSettings(InAudioData.MicrophoneDeviceIndex, InAudioData.VoiceSendVolume);
                VCS->SetNoiseSuppression(InAudioData.bNoiseSuppression);
            }
        }
    }

    if (bSaveData)
    {
        CachedSettings->Audio = InAudioData;
        InternalSave();
    }
}

void USaveManagerSubsystem::ApplyGameplay(const FGameplaySettingData& InGameplayData, bool bSaveData)
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULocalPlayer* LP = GI->GetFirstGamePlayer())
        {
            if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
            {
                VCS->SetTalkMode(InGameplayData.VOIPSetting);
            }
        }
    }

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

// ── Customization persistence ────────────────────────────────────────────────

namespace
{
    // AES-256 key (32 bytes). Change before shipping.
    constexpr uint8 GCustomizationAESKey[32] = {
        'T','r','o','m','b','o','n','e','R','u','m','b','l','e','2','0',
        '2','6','C','u','s','t','o','m','i','z','e','K','e','y','!',' '
    };
    FString GetCustomizationFilePath()
    {
        return FPaths::ProjectSavedDir() / TEXT("SaveGames") / TEXT("TromboneCustomization.dat");
    }
}

void USaveManagerSubsystem::SaveCustomization(const FCustomizationSaveData& Data)
{
    // 1. Serialize FName fields as FString (portable across sessions)
    TArray<uint8> RawBytes;
    FMemoryWriter Writer(RawBytes, true);
    FString Antenna = Data.AntennaKey.ToString();
    FString Face    = Data.FaceKey.ToString();
    FString Costume = Data.CostumeKey.ToString();
    Writer << Antenna << Face << Costume;

    // 2. Pad to AES block size (16 bytes)
    const int32 Rem = RawBytes.Num() % FAES::AESBlockSize;
    if (Rem != 0)
        RawBytes.AddZeroed(FAES::AESBlockSize - Rem);

    // 3. Encrypt in-place (AES-256)
    FAES::EncryptData(RawBytes.GetData(), (uint64)RawBytes.Num(), GCustomizationAESKey, 32);

    // 4. Base64 encode → write to file
    const FString Encoded = FBase64::Encode(RawBytes.GetData(), RawBytes.Num());
    FFileHelper::SaveStringToFile(Encoded, *GetCustomizationFilePath());

    CachedCustomization = Data;
}

FCustomizationSaveData USaveManagerSubsystem::LoadCustomization() const
{
    return CachedCustomization;
}

FCustomizationSaveData USaveManagerSubsystem::LoadCustomizationFromDisk() const
{
    FString Encoded;
    if (!FFileHelper::LoadFileToString(Encoded, *GetCustomizationFilePath()))
        return FCustomizationSaveData{};

    TArray<uint8> RawBytes;
    if (!FBase64::Decode(Encoded, RawBytes) || RawBytes.IsEmpty())
        return FCustomizationSaveData{};

    // Decrypt in-place
    FAES::DecryptData(RawBytes.GetData(), (uint64)RawBytes.Num(), GCustomizationAESKey, 32);

    // Deserialize
    FMemoryReader Reader(RawBytes, true);
    FString Antenna, Face, Costume;
    Reader << Antenna << Face << Costume;

    FCustomizationSaveData Result;
    Result.AntennaKey = FName(*Antenna);
    Result.FaceKey    = FName(*Face);
    Result.CostumeKey = FName(*Costume);
    return Result;
}

// ── Debug dump ───────────────────────────────────────────────────────────────

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
