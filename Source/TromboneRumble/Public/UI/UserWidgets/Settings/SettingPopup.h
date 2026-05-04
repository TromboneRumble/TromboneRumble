#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Popup/PopupWidgetBase.h"
#include "SettingPopup.generated.h"

class UGameplayOptionPanel;
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
	virtual void ClosePopup(bool bCloseImmediately = false) override;
	// ~ End UPopupWidgetBase Interface
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual bool NativeOnHandleBackAction() override;
	// ~ End UCommonActivatableWidget Interface
	
	bool IsAnyPanelDirty() const;
	void ShowIsDirtyNoticePopup();
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Apply;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Reset;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonAnimatedSwitcher> CAS_Settings;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAudioOptionPanel> Widget_AudioOptions;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVideoOptionPanel> Widget_VideoOptions;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULanguageOptionPanel> Widget_LanguageOptions;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGameplayOptionPanel> Widget_GameplayOptions;
	
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Audio;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Video;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Language;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Gameplay;
	
private:
	
	/** Changes Options Panel. (e.g. Audio -> Video) */
	void ChangePanel(UWidget* TargetWidget) const;
	
	void OnClickApply();
	void OnClickReset();
	
private:
	
	UPROPERTY()
	TArray<TObjectPtr<UOptionPanelBase>> OptionPanels;
	
};
