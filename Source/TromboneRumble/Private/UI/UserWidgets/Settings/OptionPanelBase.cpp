#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "CommonButtonBase.h"

void UOptionPanelBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	SaveManagerSubsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	
	Register();
	
	if (bAutoRefreshUIOnActivate)
	{
		RefreshUI();
	}
}

void UOptionPanelBase::NativeDestruct()
{
	Unregister();
	
	if (bAutoReapplySettingsOnDeactivate)
	{
		ApplySettingsFromSavedData();
	}
	
	Super::NativeDestruct();
}

void UOptionPanelBase::Register()
{
}

void UOptionPanelBase::Unregister()
{
}

void UOptionPanelBase::RefreshUI()
{
}

void UOptionPanelBase::ApplySettingsFromUI(bool bSaveToDisk)
{
}

void UOptionPanelBase::ApplySettingsFromSavedData()
{
}