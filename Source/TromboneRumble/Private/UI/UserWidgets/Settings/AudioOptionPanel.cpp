#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "AkGameplayStatics.h"
#include "Data/WwiseData.h"
#include "SaveData/TromboneSaveGame.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "Subsystems/VoiceChatSubsystem.h"
#include "Utilities/EnumHelper.h"
#include "UI/UserWidgets/Settings/SliderWidgetBase.h"
#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleRowWidget.h"
#include "UI/UserWidgets/Common/CheckBoxRowWidget.h"
#include "Engine/LocalPlayer.h"
#include "Framework/TromboneGameInstance.h"

UWidget* UAudioOptionPanel::GetFirstFocusRow() const
{
	return WBP_MasterSlider ? WBP_MasterSlider->GetFocusWidget() : nullptr;
}

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
	if (WBP_UISlider) WBP_UISlider->SetValue(AudioData.UIVolume);
	if (WBP_VoiceVolumeSlider) WBP_VoiceVolumeSlider->SetValue(AudioData.VoiceSendVolume * 0.5f); // 내 목소리 볼륨: 저장값 0~2 → 슬라이더 0~1로 정규화

	// 마이크 사이클 위젯: 저장된 장치 이름으로 매칭, 실패 시 인덱스로 폴백
	if (OC_Microphone && !CachedMicNames.IsEmpty())
	{
		int32 TargetIdx = -1;
		if (!AudioData.MicrophoneDeviceName.IsEmpty())
		{
			for (int32 i = 0; i < CachedMicNames.Num(); ++i)
			{
				if (CachedMicNames[i] == AudioData.MicrophoneDeviceName)
				{
					TargetIdx = i;
					break;
				}
			}
		}
		if (TargetIdx == -1)
		{
			TargetIdx = FMath::Clamp(AudioData.MicrophoneDeviceIndex, 0, CachedMicNames.Num() - 1);
		}
		OC_Microphone->SetSelectedIndex(TargetIdx);
	}

	if (CBR_NoiseSuppression)
	{
		CBR_NoiseSuppression->SetChecked(AudioData.bNoiseSuppression);
	}

	if (OC_VOIP)
	{
		const FGameplaySettingData GameplayData = SaveManagerSubsystem->GetGameplaySettings();
		OC_VOIP->SetSelectedIndex(EnumHelper::EnumToInt(GameplayData.VOIPSetting));
	}
}

void UAudioOptionPanel::ApplySettingsFromUI(bool bSaveToDisk)
{
	// Apply 버튼은 패널을 닫지 않으므로, 마이크 테스트를 명시적으로 중단
	if (CBR_MicTest && CBR_MicTest->IsChecked())
	{
		CBR_MicTest->SetChecked(false);
		if (UVoiceChatSubsystem* VCS = GetVCS()) VCS->EndMicTest();
	}

	Super::ApplySettingsFromUI(bSaveToDisk);

	if (bSaveToDisk)
	{
		FAudioSettingData NewAudio;
		if (WBP_MasterSlider) NewAudio.MasterVolume = WBP_MasterSlider->GetValue();
		if (WBP_BGMSlider) NewAudio.BGMVolume = WBP_BGMSlider->GetValue();
		if (WBP_MusicSlider) NewAudio.MusicVolume = WBP_MusicSlider->GetValue();
		if (WBP_SFXSlider) NewAudio.SFXVolume = WBP_SFXSlider->GetValue();
		if (WBP_UISlider) NewAudio.UIVolume = WBP_UISlider->GetValue();
		if (WBP_VoiceVolumeSlider) NewAudio.VoiceSendVolume = WBP_VoiceVolumeSlider->GetValue() * 2.0f; // 내 목소리 볼륨: 슬라이더 0~1 → 저장값 0~2
		
		if (OC_Microphone)
		{
			const int32 SelectedIdx = GetSelectedMicDeviceIndex();
			NewAudio.MicrophoneDeviceIndex = SelectedIdx;
			NewAudio.MicrophoneDeviceName = CachedMicNames.IsValidIndex(SelectedIdx)
				? CachedMicNames[SelectedIdx] : TEXT("");
		}

		NewAudio.bNoiseSuppression = CBR_NoiseSuppression ? CBR_NoiseSuppression->IsChecked() : true;

		SaveManagerSubsystem->ApplyAudio(NewAudio);

		if (OC_VOIP)
		{
			EVoipMode ParsedVoipMode;
			if (EnumHelper::IntToEnum(OC_VOIP->GetCurrentIndex(), ParsedVoipMode))
			{
				FGameplaySettingData NewGameplayData = SaveManagerSubsystem->GetGameplaySettings();
				NewGameplayData.VOIPSetting = ParsedVoipMode;
				SaveManagerSubsystem->ApplyGameplay(NewGameplayData, true);
			}
		}
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
	
	if (WBP_UISlider && !FMath::IsNearlyEqual(WBP_UISlider->GetValue(), SavedData.UIVolume, ErrorTolerance))
	{
		return true;
	}

	if (WBP_VoiceVolumeSlider &&
		!FMath::IsNearlyEqual(WBP_VoiceVolumeSlider->GetValue() * 2.0f, SavedData.VoiceSendVolume, ErrorTolerance))
	{
		return true;
	}

	// 마이크: 이름 기반 비교 (인덱스는 장치 추가/제거로 변동 가능)
	if (OC_Microphone)
	{
		const int32 SelectedIdx = OC_Microphone->GetCurrentIndex();
		const FString SelectedName = CachedMicNames.IsValidIndex(SelectedIdx)
			? CachedMicNames[SelectedIdx] : TEXT("");
		if (SelectedName != SavedData.MicrophoneDeviceName)
		{
			return true;
		}
	}

	if (CBR_NoiseSuppression && CBR_NoiseSuppression->IsChecked() != SavedData.bNoiseSuppression)
	{
		return true;
	}

	if (OC_VOIP)
	{
		EVoipMode CurrentVoipMode;
		if (EnumHelper::IntToEnum(OC_VOIP->GetCurrentIndex(), CurrentVoipMode))
		{
			if (CurrentVoipMode != SaveManagerSubsystem->GetGameplaySettings().VOIPSetting)
			{
				return true;
			}
		}
	}

	return false;
}

void UAudioOptionPanel::Register()
{
	Super::Register();

	if (WBP_MasterSlider)
	{
		WBP_MasterSlider->Init([](const float Value){ WwiseRTPC::SetVolume(WwiseRTPC::MasterVolume, Value); });
	}
	if (WBP_BGMSlider) WBP_BGMSlider->Init([](const float Value) { WwiseRTPC::SetVolume(WwiseRTPC::BGMVolume, Value); });
	if (WBP_MusicSlider) WBP_MusicSlider->Init([](const float Value) { WwiseRTPC::SetVolume(WwiseRTPC::MusicVolume, Value); });
	if (WBP_SFXSlider) WBP_SFXSlider->Init([](const float Value) { WwiseRTPC::SetVolume(WwiseRTPC::SFXVolume, Value); });
	if (WBP_UISlider) WBP_UISlider->Init([](const float Value) { WwiseRTPC::SetVolume(WwiseRTPC::UIVolume, Value); });
	if (WBP_VoiceVolumeSlider)
	{
		WBP_VoiceVolumeSlider->Init([this](const float Value)
		{
			if (UVoiceChatSubsystem* VCS = GetVCS())
			{
				//마이크 기본 옵션이 너무 작아서 임의로 늘림
				VCS->SetMicTestVolume(Value * 2000.0f);
			}
		});
	}

	PopulateMicCycleWidget();

	if (OC_Microphone)
	{
		OC_Microphone->OnRotatedWithDirection().RemoveAll(this);
		OC_Microphone->OnRotatedWithDirection().AddDynamic(this, &ThisClass::OnMicRotated);
	}

	if (CBR_MicTest)
	{
		CBR_MicTest->SetChecked(false); // 패널 열릴 때 항상 비활성화 상태로 시작
		CBR_MicTest->OnClicked().RemoveAll(this);
		CBR_MicTest->OnClicked().AddUObject(this, &UAudioOptionPanel::OnMicTestToggled);
	}

	if (CBR_NoiseSuppression)
	{
		if (SaveManagerSubsystem)
		{
			CBR_NoiseSuppression->SetChecked(SaveManagerSubsystem->GetAudioSettings().bNoiseSuppression);
		}
		CBR_NoiseSuppression->OnClicked().RemoveAll(this);
		CBR_NoiseSuppression->OnClicked().AddUObject(this, &UAudioOptionPanel::OnNoiseSuppressionToggled);
	}
}

void UAudioOptionPanel::Unregister()
{
	// 마이크 테스트 중이면 패널 닫힐 때 강제 중단
	if (UVoiceChatSubsystem* VCS = GetVCS())
	{
		VCS->EndMicTest();
	}

	if (OC_Microphone)
	{
		OC_Microphone->OnRotatedWithDirection().RemoveAll(this);
	}

	if (CBR_MicTest)
	{
		CBR_MicTest->OnClicked().RemoveAll(this);
	}

	if (CBR_NoiseSuppression)
	{
		CBR_NoiseSuppression->OnClicked().RemoveAll(this);
	}

	Super::Unregister();

	if (WBP_MasterSlider) WBP_MasterSlider->Init(nullptr);
	if (WBP_BGMSlider) WBP_BGMSlider->Init(nullptr);
	if (WBP_MusicSlider) WBP_MusicSlider->Init(nullptr);
	if (WBP_SFXSlider) WBP_SFXSlider->Init(nullptr);
	if (WBP_UISlider) WBP_UISlider->Init(nullptr);
	if (WBP_VoiceVolumeSlider) WBP_VoiceVolumeSlider->Init(nullptr);
}

UVoiceChatSubsystem* UAudioOptionPanel::GetVCS() const
{
	const ULocalPlayer* LP = GetOwningLocalPlayer();
	return LP ? LP->GetSubsystem<UVoiceChatSubsystem>() : nullptr;
}

void UAudioOptionPanel::PopulateMicCycleWidget()
{
	if (!OC_Microphone) return;

	UVoiceChatSubsystem* VCS = GetVCS();
	if (!VCS) return;

	CachedMicNames = VCS->GetAvailableMicDeviceNames();

	TArray<FText> Options;
	for (const FString& Name : CachedMicNames)
	{
		Options.Add(FText::FromString(Name));
	}

	const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance());
	const FText Title = GI
		? GI->GetCommonUIText(TEXT("Setting_Microphone"))
		: FText::FromString(TEXT("Mic"));

	OC_Microphone->ForceInit(Title, Options, 0);
}

int32 UAudioOptionPanel::GetSelectedMicDeviceIndex() const
{
	return OC_Microphone ? OC_Microphone->GetCurrentIndex() : 0;
}

void UAudioOptionPanel::OnMicRotated(int32 NewIndex, ERotatorDirection RotatorDir)
{
	if (UVoiceChatSubsystem* VCS = GetVCS())
	{
		if (VCS->IsMicTesting())
		{
			VCS->EndMicTest();
			if (CBR_MicTest) CBR_MicTest->SetChecked(false);
		}
	}
}

void UAudioOptionPanel::OnNoiseSuppressionToggled()
{
	if (!CBR_NoiseSuppression) return;
	if (UVoiceChatSubsystem* VCS = GetVCS())
	{
		VCS->SetNoiseSuppression(CBR_NoiseSuppression->IsChecked());
	}
}

void UAudioOptionPanel::OnMicTestToggled()
{
	if (!CBR_MicTest) return;

	// CB_CheckBox가 Toggleable로 설정된 경우, 클릭 후 IsChecked()는 이미 새 상태를 반영
	const bool bIsChecked = CBR_MicTest->IsChecked();
	if (UVoiceChatSubsystem* VCS = GetVCS())
	{
		if (bIsChecked)
		{
			VCS->BeginMicTest(GetSelectedMicDeviceIndex());
			// BeginMicTest 이후 bMicTestActive=true가 되므로 즉시 현재 슬라이더 볼륨 적용
			if (WBP_VoiceVolumeSlider)
			{
				VCS->SetMicTestVolume(WBP_VoiceVolumeSlider->GetValue() * 2000.0f);
			}
		}
		else
		{
			VCS->EndMicTest();
		}
	}
}
