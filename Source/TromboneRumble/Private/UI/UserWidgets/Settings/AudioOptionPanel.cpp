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

void UAudioOptionPanel::ReapplySavedSettings()
{
	Super::ReapplySavedSettings();
	
	if (!SaveManagerSubsystem)
	{
		return;
	}
	
	const FAudioSettingData AudioData = SaveManagerSubsystem->GetAudioSettings();
	
	SaveManagerSubsystem->ApplyAudio(AudioData);
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

	RefreshUI();
	ReapplySavedSettings();
}