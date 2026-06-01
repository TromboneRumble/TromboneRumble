// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Lobby/VoiceVolumeRowWidget.h"
#include "AnalogSlider.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Framework/DefaultPlayerState.h"
#include "Subsystems/VoiceChatSubsystem.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "SaveData/TromboneSaveGame.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

void UVoiceVolumeRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Slider)
	{
		Slider->OnValueChanged.AddDynamic(this, &ThisClass::HandleSliderValueChanged);
	}
}

void UVoiceVolumeRowWidget::NativeDestruct()
{
	if (Slider)
	{
		Slider->OnValueChanged.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UVoiceVolumeRowWidget::Init(ADefaultPlayerState* InPS, bool bIsLocal)
{
	WeakPS = InPS;
	bIsLocalPlayerRow = bIsLocal;
	if (!Slider || !InPS) return;

	float InitValue = 0.5f;
	if (bIsLocal)
	{
		if (USaveManagerSubsystem* SM = GetSaveManager())
		{
			InitValue = SM->GetAudioSettings().VoiceSendVolume * 0.5f;
		}
	}
	else if (UVoiceChatSubsystem* VCS = GetVCS())
	{
		InitValue = VCS->GetRemotePlayerVolume(InPS) * 0.5f;
	}

	Slider->SetValue(InitValue);
	if (ProgressBar)
	{
		ProgressBar->SetPercent(InitValue);
	}
	if (Text_Value)
	{
		Text_Value->SetText(FText::AsNumber(FMath::RoundToInt(InitValue * 100.0f)));
	}
}

void UVoiceVolumeRowWidget::HandleSliderValueChanged(float Value)
{
	if (ProgressBar)
	{
		ProgressBar->SetPercent(Value);
	}
	if (Text_Value)
	{
		Text_Value->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100.0f)));
	}

	ADefaultPlayerState* PS = WeakPS.Get();
	if (!PS) return;

	if (bIsLocalPlayerRow)
	{
		if (USaveManagerSubsystem* SM = GetSaveManager())
		{
			FAudioSettingData AudioData = SM->GetAudioSettings();
			AudioData.VoiceSendVolume = Value * 2.0f;
			SM->ApplyAudio(AudioData, true);
		}
	}
	else
	{
		if (UVoiceChatSubsystem* VCS = GetVCS())
		{
			VCS->SetRemotePlayerVolume(PS, Value * 2.0f);
		}
	}
}

UVoiceChatSubsystem* UVoiceVolumeRowWidget::GetVCS() const
{
	const APlayerController* PC = GetOwningPlayer();
	if (!PC) return nullptr;
	const ULocalPlayer* LP = PC->GetLocalPlayer();
	return LP ? LP->GetSubsystem<UVoiceChatSubsystem>() : nullptr;
}

USaveManagerSubsystem* UVoiceVolumeRowWidget::GetSaveManager() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<USaveManagerSubsystem>() : nullptr;
}
