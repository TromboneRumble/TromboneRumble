#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Popup/PopupBase.h"
#include "SettingPopup.generated.h"

class UCommonButtonGroupBase;
class UGameplayOptionPanel;
class UOptionPanelBase;
class ULanguageOptionPanel;
class UCommonAnimatedSwitcher;
class UCommonButtonBase;
class UVideoOptionPanel;
class UAudioOptionPanel;

UCLASS()
class TROMBONERUMBLE_API USettingPopup : public UPopupBase
{
	GENERATED_BODY()
	
public:

	// ~ Begin UPopupWidget Interface
	virtual void ClosePopup(const bool bCloseImmediately = false) override;
	// ~ End UPopupWidget Interface
	
protected:
	
	// ~ Begin UPopupWidget Interface
	virtual void Register() override;
	virtual void Unregister() override;
	// ~ End UPopupWidget Interface
	
	// ~ Begin UCommonActivatableWidget Interface
	virtual void NativeConstruct() override;
	virtual bool NativeOnHandleBackAction() override;
	// ~ End UCommonActivatableWidget Interface
	
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
	
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Audio;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Video;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Language;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CB_Gameplay;
	
private:
	
	/** Changes Options Panel. (e.g. Audio -> Video) */
	void ChangePanel(UWidget* TargetWidget) const;
	
	void OnClickApply();
	void OnClickReset();
	
	bool IsAnyPanelDirty() const;
	void ShowIsDirtyNoticePopup();
	
private:
	
	UPROPERTY()
	TArray<TObjectPtr<UOptionPanelBase>> OptionPanels;
	
	UPROPERTY()
	TObjectPtr<UCommonButtonGroupBase> CategoryButtonGroup;
	
};
