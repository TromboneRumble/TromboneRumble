#include "UI/UserWidgets/Settings/SettingPopup.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "UI/UserWidgets/Settings/VideoOptionPanel.h"
#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"

void USettingPopup::NativeConstruct()
{
	Super::NativeConstruct();
	
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
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddLambda([this]()
		{
			if (UOptionPanelBase* ActivePanel = Cast<UOptionPanelBase>(CAS_Settings->GetActiveWidget()))
			{
				ActivePanel->Deactivate();
			}
			ClosePopup();
		});
	}
}

void USettingPopup::ChangePanel(UWidget* TargetWidget) const
{
	if (CAS_Settings)
	{
		CAS_Settings->SetActiveWidget(TargetWidget);
	}	
}
