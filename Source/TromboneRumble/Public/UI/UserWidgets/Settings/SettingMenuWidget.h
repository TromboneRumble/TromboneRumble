#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "SettingMenuWidget.generated.h"

class ULanguageOptionPanel;
class UCommonAnimatedSwitcher;
class UCommonButtonBase;
class UVideoOptionPanel;
class UAudioOptionPanel;

UCLASS()
class TROMBONERUMBLE_API USettingMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()
	
protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	
	virtual void Init() override;
	
private:
	void ChangePanel(UWidget* TargetWidget);
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonAnimatedSwitcher> CAS_Settings;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UAudioOptionPanel> Widget_AudioOptions;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVideoOptionPanel> Widget_VideoOptions;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<ULanguageOptionPanel> Widget_LanguageOptions;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Audio;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Video;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Language;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Back;
};
