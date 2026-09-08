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
	virtual UWidget* GetDefaultFocusWidget() const override;
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
	
	/** LB row in DT_Input. Rows eat Left / Right, so the tabs need their own keys. */
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle PrevTabActionRow;
	
	/** RB row in DT_Input. */
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle NextTabActionRow;
	
private:
	
	/** Changes Options Panel. (e.g. Audio -> Video) */
	void ChangePanel(UWidget* TargetWidget) const;
	
	/** Called when the tab group selects a tab, by click or by LB / RB. Shows that tab's panel. */
	void HandleTabSelected(UCommonButtonBase* Button, int32 Index);
	
	/** Called when the switcher starts showing a panel. Gives gamepad focus to its first row. */
	void HandleActivePanelChanged(UWidget* ActiveWidget, int32 Index);
	
	void OnClickApply();
	void OnClickReset();
	
	bool IsAnyPanelDirty() const;
	void ShowUnsavedChangesPopup();
	
private:
	
	UPROPERTY()
	TArray<TObjectPtr<UOptionPanelBase>> OptionPanels;
	
	UPROPERTY()
	TObjectPtr<UCommonButtonGroupBase> CategoryButtonGroup;
	
	FUIActionBindingHandle PrevTabActionHandle;
	FUIActionBindingHandle NextTabActionHandle;
	
};
