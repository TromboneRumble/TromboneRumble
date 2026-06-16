// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenu/VoiceVolumeRowWidget.h"
#include "AnalogSlider.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "TimerManager.h"
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
	if (Btn_Plus)
	{
		Btn_Plus->OnPressed.AddDynamic(this, &ThisClass::HandlePlusPressed);
		Btn_Plus->OnReleased.AddDynamic(this, &ThisClass::HandleButtonReleased);
	}
	if (Btn_Minus)
	{
		Btn_Minus->OnPressed.AddDynamic(this, &ThisClass::HandleMinusPressed);
		Btn_Minus->OnReleased.AddDynamic(this, &ThisClass::HandleButtonReleased);
	}
}

void UVoiceVolumeRowWidget::NativeDestruct()
{
	if (Slider)
	{
		Slider->OnValueChanged.RemoveAll(this);
	}
	if (Btn_Plus)
	{
		Btn_Plus->OnPressed.RemoveAll(this);
		Btn_Plus->OnReleased.RemoveAll(this);
	}
	if (Btn_Minus)
	{
		Btn_Minus->OnPressed.RemoveAll(this);
		Btn_Minus->OnReleased.RemoveAll(this);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HoldDelayTimerHandle);
		World->GetTimerManager().ClearTimer(HoldRepeatTimerHandle);
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
		Text_Value->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(InitValue * 200.0f))));
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
		Text_Value->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 200.0f))));
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

void UVoiceVolumeRowWidget::HandlePlusPressed()
{
	StartHold(+1);
}

void UVoiceVolumeRowWidget::HandleMinusPressed()
{
	StartHold(-1);
}

void UVoiceVolumeRowWidget::HandleButtonReleased()
{
	StopHold();
}

void UVoiceVolumeRowWidget::StartHold(int32 Direction)
{
	HoldDirection = Direction;
	// 단일 클릭(또는 꾹의 첫 스텝): 즉시 1% 적용 → 짧게 탭하면 정확히 1%
	ApplyDelta(Direction * (ClickStepPercent / 200.0f));
	ContinuousElapsed = 0.0f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HoldDelayTimerHandle, this,
			&ThisClass::BeginContinuousAdjust, HoldStartDelay, false);
	}
}

void UVoiceVolumeRowWidget::BeginContinuousAdjust()
{
	ContinuousElapsed = 0.0f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HoldRepeatTimerHandle, this,
			&ThisClass::HandleHoldRepeat, RepeatInterval, true);
	}
}

void UVoiceVolumeRowWidget::HandleHoldRepeat()
{
	ContinuousElapsed += RepeatInterval;
	// 지수 가속: speed = Min * Growth^t, Max로 clamp
	const float SpeedPercentPerSec = FMath::Min(
		MinHoldSpeedPercent * FMath::Pow(HoldAccelGrowth, ContinuousElapsed),
		MaxHoldSpeedPercent);
	const float DeltaSlider = HoldDirection * (SpeedPercentPerSec / 200.0f) * RepeatInterval;
	ApplyDelta(DeltaSlider);
}

void UVoiceVolumeRowWidget::StopHold()
{
	HoldDirection = 0;
	ContinuousElapsed = 0.0f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HoldDelayTimerHandle);
		World->GetTimerManager().ClearTimer(HoldRepeatTimerHandle);
	}
}

void UVoiceVolumeRowWidget::ApplyDelta(float DeltaSliderValue)
{
	if (!Slider) return;
	const float NewValue = FMath::Clamp(Slider->GetValue() + DeltaSliderValue, 0.0f, 1.0f);
	Slider->SetValue(NewValue);  // OnValueChanged broadcast → HandleSliderValueChanged 자동 호출
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
