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
	virtual void RefreshUI() override;
	
protected:
	virtual void Activate() override;
	virtual void Deactivate() override;
	
	virtual void Register() override;
	
	virtual void HandleApplyButtonClicked() override;
	virtual void HandleResetButtonClicked() override;
	
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