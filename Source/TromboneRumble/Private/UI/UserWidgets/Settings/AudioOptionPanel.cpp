// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "AkGameplayStatics.h"
#include "CommonTextBlock.h"
#include "SaveData/TromboneSaveGame.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Settings/SliderWidgetBase.h"

void UAudioOptionPanel::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	Text_OptionPanelTitle->SetText(FText::FromString(TEXT("오디오 옵션")));
}

void UAudioOptionPanel::Init(const TFunction<void()> BackAction)
{
	Super::Init(BackAction);
	
	InitSliders();
	
	if (SaveManagerSubsystem)
	{
		if (const UTromboneSaveGame* SavedSettings = SaveManagerSubsystem->GetCurrentCustomSettings())
		{
			UpdateUIFromSettings(SavedSettings->Audio);
		}
	}
}

void UAudioOptionPanel::HandleBackButtonClicked()
{
	Super::HandleBackButtonClicked();
	
	SaveManagerSubsystem->InitializeSettings();
}

void UAudioOptionPanel::HandleApplyButtonClicked()
{
	Super::HandleApplyButtonClicked();
	
	FAudioSettingData NewAudio;
	NewAudio.MasterVolume = WBP_MasterSlider->GetValue();
	NewAudio.MusicVolume = WBP_MusicSlider->GetValue();
	NewAudio.SFXVolume = WBP_SFXSlider->GetValue();

	SaveManagerSubsystem->SaveAudioSettings(NewAudio);
}

void UAudioOptionPanel::HandleResetButtonClicked()
{
	Super::HandleResetButtonClicked();
	
	SaveManagerSubsystem->InitializeSettings();
}

void UAudioOptionPanel::InitSliders() const
{
	WBP_MasterSlider->Init([this](const float Value)
	{
		UAkGameplayStatics::SetRTPCValue(nullptr, Value * 100.f, 0, nullptr, FName(TEXT("RTPC_MasterVolume")));
	});
	
	WBP_MusicSlider->Init([this](const float Value)
	{
		UAkGameplayStatics::SetRTPCValue(nullptr, Value * 100.f, 0, nullptr, FName(TEXT("RTPC_MusicVolume")));
	});
	WBP_SFXSlider->Init([this](const float Value)
	{
		UAkGameplayStatics::SetRTPCValue(nullptr, Value * 100.f, 0, nullptr, FName(TEXT("RTPC_SFXVolume")));
	});
}

void UAudioOptionPanel::UpdateUIFromSettings(const FAudioSettingData& AudioData) const
{
	if (WBP_MasterSlider) WBP_MasterSlider->SetValue(AudioData.MasterVolume);
	if (WBP_MusicSlider) WBP_MusicSlider->SetValue(AudioData.MusicVolume);
	if (WBP_SFXSlider) WBP_SFXSlider->SetValue(AudioData.SFXVolume);
}