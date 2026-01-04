// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "AkGameplayStatics.h"
#include "CommonTextBlock.h"
#include "Components/Slider.h"
#include "SaveData/TromboneSaveGame.h"
#include "Subsystems/SaveManagerSubsystem.h"

void UAudioOptionPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->OnValueChanged.AddDynamic(this, &ThisClass::OnMasterVolumeChanged);
	}
	if (Slider_MusicVolume)
	{
		Slider_MusicVolume->OnValueChanged.AddDynamic(this, &ThisClass::OnMusicVolumeChanged);
	}
}

void UAudioOptionPanel::Init(TFunction<void()> BackAction)
{
	Super::Init(BackAction);
	
	Text_OptionPanelTitle->SetText(FText::FromString(TEXT("Audio Options")));
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
	NewAudio.MasterVolume = Slider_MasterVolume->GetValue();
	NewAudio.MusicVolume = Slider_MusicVolume->GetValue();
	NewAudio.SFXVolume = Slider_SFXVolume->GetValue();

	SaveManagerSubsystem->SaveAudioSettings(NewAudio);
}

void UAudioOptionPanel::HandleResetButtonClicked()
{
	Super::HandleResetButtonClicked();
	
	SaveManagerSubsystem->InitializeSettings();
}

void UAudioOptionPanel::OnMasterVolumeChanged(float Value)
{
	UAkGameplayStatics::SetRTPCValue(nullptr, Value * 100.f, 0, nullptr, FName(TEXT("RTPC_MasterVolume")));
}

void UAudioOptionPanel::OnMusicVolumeChanged(float Value)
{
	UAkGameplayStatics::SetRTPCValue(nullptr, Value * 100.f, 0, nullptr, FName(TEXT("RTPC_MusicVolume")));
}