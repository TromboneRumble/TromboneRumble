#include "UI/UserWidgets/Settings/SettingPopup.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "Framework/TromboneGameInstance.h"
#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "UI/UserWidgets/Settings/GameplayOptionPanel.h"
#include "UI/UserWidgets/Settings/VideoOptionPanel.h"
#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"
#include "Utilities/TromboneStatics.h"

void USettingPopup::ChangePanel(UWidget* TargetWidget) const
{
	if (CAS_Settings)
	{
		CAS_Settings->SetActiveWidget(TargetWidget);
	}	
}

void USettingPopup::OnClickApply()
{
	for (const auto& Panel : OptionPanels)
	{
		if (Panel)
		{
			Panel->ApplySettingsFromUI(true);
		}
	}
}

void USettingPopup::OnClickReset()
{
	for (const auto& Panel : OptionPanels)
	{
		if (Panel)
		{
			Panel->ApplySettingsFromSavedData();
			Panel->RefreshUI();
		}
	}
}

void USettingPopup::Register()
{
	Super::Register();
	
	if (CB_Audio)
	{
		CB_Audio->OnClicked().RemoveAll(this);
		CB_Audio->OnClicked().AddLambda([this] { ChangePanel(Widget_AudioOptions); });
	}
	if (CB_Video)
	{
		CB_Video->OnClicked().RemoveAll(this);
		CB_Video->OnClicked().AddLambda([this] { ChangePanel(Widget_VideoOptions); });
	}
	if (CB_Language)
	{
		CB_Language->OnClicked().RemoveAll(this);
		CB_Language->OnClicked().AddLambda([this] { ChangePanel(Widget_LanguageOptions); });
	}
	if (CB_Gameplay)
	{
		CB_Gameplay->OnClicked().RemoveAll(this);
		CB_Gameplay->OnClicked().AddLambda([this] { ChangePanel(Widget_GameplayOptions); });
	}
	
	if (Button_Apply)
	{
		Button_Apply->OnClicked().RemoveAll(this);
		Button_Apply->OnClicked().AddUObject(this, &USettingPopup::OnClickApply);
	}
	
	if (Button_Reset)
	{
		Button_Reset->OnClicked().RemoveAll(this);
		Button_Reset->OnClicked().AddUObject(this, &USettingPopup::OnClickReset);
	}
	
	OptionPanels = { Widget_AudioOptions, Widget_VideoOptions, Widget_LanguageOptions, Widget_GameplayOptions };
}

void USettingPopup::Unregister()
{
	Super::Unregister();
	
	if (CB_Audio)
	{
		CB_Audio->OnClicked().RemoveAll(this);
	}
	if (CB_Video)
	{
		CB_Video->OnClicked().RemoveAll(this);
	}
	if (CB_Language)
	{
		CB_Language->OnClicked().RemoveAll(this);
	}
	if (CB_Gameplay)
	{
		CB_Gameplay->OnClicked().RemoveAll(this);
	}
	if (Button_Apply)
	{
		Button_Apply->OnClicked().RemoveAll(this);
	}
	if (Button_Reset)
	{
		Button_Reset->OnClicked().RemoveAll(this);
	}
}

void USettingPopup::ClosePopup(bool bCloseImmediately)
{
	if (IsAnyPanelDirty() && GetWorld())
	{
		ShowIsDirtyNoticePopup();
		return;
	}
	
	Super::ClosePopup(bCloseImmediately);
}

bool USettingPopup::NativeOnHandleBackAction()
{
	if (IsAnyPanelDirty() && GetWorld())
	{
		ShowIsDirtyNoticePopup();
		return true;
	}
	
	return Super::NativeOnHandleBackAction();
}

bool USettingPopup::IsAnyPanelDirty() const
{
	for (const auto& Panel : OptionPanels)
	{
		if (Panel && Panel->IsDirty())
		{
			return true;
		}
	}
	
	return false;
}

void USettingPopup::ShowIsDirtyNoticePopup()
{
	// call parent ClosePopup()
	UPopupWidgetBase::ClosePopup(true);
	
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{		
		const FText Title = FText::FromString(TEXT(""));
		const FText Description = GI->GetCommonUIText(TEXT("SettingPopup_AskConfirmation"));
		const FText LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
		const FText RightButtonText = GI->GetCommonUIText(TEXT("Common_No"));
		
		UTwoButtonWithoutClosePopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonWithoutClosePopup>(GetWorld());

		const TFunction<void()> LeftCallback = [this, Popup]()
		{
			OnClickApply();
			Popup->ClosePopup();
		};

		const TFunction<void()> RightCallback = [this, Popup]()
		{
			OnClickReset();
			Popup->ClosePopup();
		};
		
		Popup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftCallback, RightCallback);
	}
}
