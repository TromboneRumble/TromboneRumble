// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OptionPanelBase.h"
#include "AudioOptionPanel.generated.h"

class USliderWidgetBase;
struct FAudioSettingData;
class USlider;

UCLASS()
class TROMBONERUMBLE_API UAudioOptionPanel : public UOptionPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void Init(TFunction<void()> BackAction) override;
	
protected:
	virtual void HandleBackButtonClicked() override;
	virtual void HandleApplyButtonClicked() override;
	virtual void HandleResetButtonClicked() override;
	
private:
	void InitSliders() const;
	void UpdateUIFromSettings(const FAudioSettingData& AudioData) const;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_MasterSlider;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_MusicSlider;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USliderWidgetBase> WBP_SFXSlider;
};