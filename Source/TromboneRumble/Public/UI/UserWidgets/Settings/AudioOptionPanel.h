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
	
protected:
	
	// ~ Begin UOptionPanelBase Interface
	virtual void RefreshUI() override;
	virtual void ReapplySavedSettings() override;
	virtual void Register() override;
	virtual void HandleApplyButtonClicked() override;
	virtual void HandleResetButtonClicked() override;
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
	
};