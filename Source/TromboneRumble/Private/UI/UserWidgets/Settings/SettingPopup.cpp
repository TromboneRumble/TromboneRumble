#include "UI/UserWidgets/Settings/SettingPopup.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "UI/UserWidgets/Settings/VideoOptionPanel.h"
#include "UI/UserWidgets/Settings/LanguageOptionPanel.h"

void USettingPopup::ChangePanel(UWidget* TargetWidget) const
{
	if (CAS_Settings)
	{
		CAS_Settings->SetActiveWidget(TargetWidget);
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
}
