#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "AkGameplayStatics.h"
#include "Data/WwiseData.h"
#include "SaveData/TromboneSaveGame.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Settings/SliderWidgetBase.h"

void UAudioOptionPanel::RefreshUI()
{
	Super::RefreshUI();
	
	if (!SaveManagerSubsystem)
	{
		return;
	}
	
	const FAudioSettingData AudioData = SaveManagerSubsystem->GetAudioSettings();
	
	if (WBP_MasterSlider) WBP_MasterSlider->SetValue(AudioData.MasterVolume);
	if (WBP_BGMSlider) WBP_BGMSlider->SetValue(AudioData.BGMVolume);
	if (WBP_MusicSlider) WBP_MusicSlider->SetValue(AudioData.MusicVolume);
	if (WBP_SFXSlider) WBP_SFXSlider->SetValue(AudioData.SFXVolume);
}

void UAudioOptionPanel::ApplySettingsFromUI(bool bSaveToDisk)
{
	Super::ApplySettingsFromUI(bSaveToDisk);
	
	if (bSaveToDisk)
	{
		FAudioSettingData NewAudio;
		NewAudio.MasterVolume = WBP_MasterSlider->GetValue();
		NewAudio.BGMVolume = WBP_BGMSlider->GetValue();
		NewAudio.MusicVolume = WBP_MusicSlider->GetValue();
		NewAudio.SFXVolume = WBP_SFXSlider->GetValue();

		SaveManagerSubsystem->ApplyAudio(NewAudio);
	}
}

void UAudioOptionPanel::ApplySettingsFromSavedData()
{
	Super::ApplySettingsFromSavedData();
	
	if (!SaveManagerSubsystem)
	{
		return;
	}
	
	const FAudioSettingData AudioData = SaveManagerSubsystem->GetAudioSettings();
	
	SaveManagerSubsystem->ApplyAudio(AudioData, false);
}

bool UAudioOptionPanel::IsDirty() const
{
	if (Super::IsDirty())
	{
		return true;
	}

	if (!SaveManagerSubsystem)
	{
		return false;
	}

	const FAudioSettingData SavedData = SaveManagerSubsystem->GetAudioSettings();
	constexpr float ErrorTolerance = 0.001f;

	if (WBP_MasterSlider && !FMath::IsNearlyEqual(WBP_MasterSlider->GetValue(), SavedData.MasterVolume, ErrorTolerance))
	{
		return true;
	}

	if (WBP_BGMSlider && !FMath::IsNearlyEqual(WBP_BGMSlider->GetValue(), SavedData.BGMVolume, ErrorTolerance))
	{
		return true;
	}

	if (WBP_MusicSlider && !FMath::IsNearlyEqual(WBP_MusicSlider->GetValue(), SavedData.MusicVolume, ErrorTolerance))
	{
		return true;
	}

	if (WBP_SFXSlider && !FMath::IsNearlyEqual(WBP_SFXSlider->GetValue(), SavedData.SFXVolume, ErrorTolerance))
	{
		return true;
	}

	return false;
}

void UAudioOptionPanel::Register()
{
	Super::Register();
	
	WBP_MasterSlider->Init([this](const float Value)
	{
		WwiseRTPC::SetVolume(WwiseRTPC::MasterVolume, Value);
	});
	WBP_BGMSlider->Init([this](const float Value)
	{
		WwiseRTPC::SetVolume(WwiseRTPC::BGMVolume, Value);
	});
	WBP_MusicSlider->Init([this](const float Value)
	{
		WwiseRTPC::SetVolume(WwiseRTPC::MusicVolume, Value);
	});
	WBP_SFXSlider->Init([this](const float Value)
	{
		WwiseRTPC::SetVolume(WwiseRTPC::SFXVolume, Value);
	});
}

void UAudioOptionPanel::Unregister()
{
	Super::Unregister();
	
	WBP_MasterSlider->Init(nullptr);
	WBP_BGMSlider->Init(nullptr);
	WBP_MusicSlider->Init(nullptr);
	WBP_SFXSlider->Init(nullptr);
}