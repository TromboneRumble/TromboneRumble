#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "AkGameplayStatics.h"
#include "Data/WwiseData.h"
#include "SaveData/TromboneSaveGame.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Settings/SliderWidgetBase.h"

void UAudioOptionPanel::Init()
{
	Super::Init();
	
	InitSliders();
	
	if (SaveManagerSubsystem)
	{
		if (const UTromboneSaveGame* SavedSettings = SaveManagerSubsystem->LoadOrCreateSettings())
		{
			UpdateUIFromSettings(SavedSettings->Audio);
		}
	}
}

void UAudioOptionPanel::HandleDeactivated()
{
	Super::HandleDeactivated();
	
	const FAudioSettingData Data = SaveManagerSubsystem->GetAudioSettings();
	UpdateUIFromSettings(Data);
	SaveManagerSubsystem->ApplyAudio(Data);
}

void UAudioOptionPanel::HandleApplyButtonClicked()
{
	Super::HandleApplyButtonClicked();
	
	FAudioSettingData NewAudio;
	NewAudio.MasterVolume = WBP_MasterSlider->GetValue();
	NewAudio.BGMVolume = WBP_BGMSlider->GetValue();
	NewAudio.MusicVolume = WBP_MusicSlider->GetValue();
	NewAudio.SFXVolume = WBP_SFXSlider->GetValue();

	SaveManagerSubsystem->ApplyAndSaveAudio(NewAudio);
}

void UAudioOptionPanel::HandleResetButtonClicked()
{
	Super::HandleResetButtonClicked();

	const FAudioSettingData Data = SaveManagerSubsystem->GetAudioSettings();
	UpdateUIFromSettings(Data);
	SaveManagerSubsystem->ApplyAudio(Data);
}

void UAudioOptionPanel::InitSliders() const
{
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

void UAudioOptionPanel::UpdateUIFromSettings(const FAudioSettingData& AudioData) const
{
	if (WBP_MasterSlider) WBP_MasterSlider->SetValue(AudioData.MasterVolume);
	if (WBP_BGMSlider) WBP_BGMSlider->SetValue(AudioData.BGMVolume);
	if (WBP_MusicSlider) WBP_MusicSlider->SetValue(AudioData.MusicVolume);
	if (WBP_SFXSlider) WBP_SFXSlider->SetValue(AudioData.SFXVolume);
	
	WwiseRTPC::SetVolume(WwiseRTPC::MasterVolume, AudioData.MasterVolume);
	WwiseRTPC::SetVolume(WwiseRTPC::BGMVolume, AudioData.BGMVolume);
	WwiseRTPC::SetVolume(WwiseRTPC::MusicVolume, AudioData.MusicVolume);
	WwiseRTPC::SetVolume(WwiseRTPC::SFXVolume, AudioData.SFXVolume);
}