#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Popup/PopupWidgetBase.h"
#include "SettingPopup.generated.h"

class UOptionPanelBase;
class ULanguageOptionPanel;
class UCommonAnimatedSwitcher;
class UCommonButtonBase;
class UVideoOptionPanel;
class UAudioOptionPanel;

UCLASS()
class TROMBONERUMBLE_API USettingPopup : public UPopupWidgetBase
{
	GENERATED_BODY()

protected:
	
	// ~ Begin UPopupWidgetBase Interface
	virtual void Register() override;
	virtual void Unregister() override;
	// ~ End UPopupWidgetBase Interface
	
private:
	
	/** Changes Options Panel. (e.g. Audio -> Video) */
	void ChangePanel(UWidget* TargetWidget) const;
	
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonAnimatedSwitcher> CAS_Settings;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UAudioOptionPanel> Widget_AudioOptions;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVideoOptionPanel> Widget_VideoOptions;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<ULanguageOptionPanel> Widget_LanguageOptions;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UOptionPanelBase> Widget_GameplayOptions;
	
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Audio;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Video;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Language;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Gameplay;
	
};
