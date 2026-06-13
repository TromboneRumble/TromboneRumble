#pragma once

#include "CoreMinimal.h"
#include "OptionPanelBase.h"
#include "AudioOptionPanel.generated.h"

class USliderWidgetBase;
struct FAudioSettingData;
class UVoiceChatSubsystem;
class UOptionCycleRowWidget;
class UCheckBoxRowWidget;
enum class ERotatorDirection : uint8;

UCLASS()
class TROMBONERUMBLE_API UAudioOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()

public:

	// ~ Begin UOptionPanelBase Interface
	virtual void RefreshUI() override;
	virtual void ApplySettingsFromUI(bool bSaveToDisk) override;
	virtual void ApplySettingsFromSavedData() override;
	virtual bool IsDirty() const override;
	// ~ End UOptionPanelBase Interface

protected:

	// ~ Begin UOptionPanelBase Interface
	virtual void Register() override;
	virtual void Unregister() override;
	// ~ End UOptionPanelBase Interface

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_MasterSlider;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_BGMSlider;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_MusicSlider;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_SFXSlider;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_UISlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOptionCycleRowWidget> OC_Microphone;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_VoiceVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBoxRowWidget> CBR_MicTest;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBoxRowWidget> CBR_NoiseSuppression;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOptionCycleRowWidget> OC_VOIP;

	TArray<FString> CachedMicNames;

	void PopulateMicCycleWidget();
	UVoiceChatSubsystem* GetVCS() const;
	int32 GetSelectedMicDeviceIndex() const;

	UFUNCTION()
	void OnMicRotated(int32 NewIndex, ERotatorDirection RotatorDir);

	void OnMicTestToggled();
	void OnNoiseSuppressionToggled();

};
