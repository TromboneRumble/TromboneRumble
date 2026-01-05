// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OptionPanelBase.h"
#include "AudioOptionPanel.generated.h"

struct FAudioSettingData;
class USlider;

UCLASS()
class TROMBONERUMBLE_API UAudioOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void Init(TFunction<void()> BackAction) override;
	
protected:
	virtual void HandleBackButtonClicked() override;
	virtual void HandleApplyButtonClicked() override;
	virtual void HandleResetButtonClicked() override;
	
private:
	UFUNCTION()
	void OnMasterVolumeChanged(float Value);
	UFUNCTION()
	void OnMusicVolumeChanged(float Value);
	
	void UpdateUIFromSettings(const FAudioSettingData& AudioData);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_MasterVolume;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_MusicVolume;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_SFXVolume;
};
