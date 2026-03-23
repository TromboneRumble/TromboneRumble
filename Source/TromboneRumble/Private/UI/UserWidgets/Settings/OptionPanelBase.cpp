#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "CommonButtonBase.h"

void UOptionPanelBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	InitButtons();
}

void UOptionPanelBase::Init()
{
	SaveManagerSubsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
}

void UOptionPanelBase::InitButtons()
{
	if (Button_Apply)
	{
		Button_Apply->OnClicked().AddUObject(this, &ThisClass::HandleApplyButtonClicked);
	}
	if (Button_Reset)
	{
		Button_Reset->OnClicked().AddUObject(this, &ThisClass::HandleResetButtonClicked);
	}
}

void UOptionPanelBase::HandleDeactivated()
{
	// To be overridden by child classes if needed
	// TODO : SettingMenuWidget에 이벤트 만들어서 적용, 리셋 이벤트 관리하고, Back으로 나갈때도 이벤트 관리해야 함 
	
}

void UOptionPanelBase::HandleApplyButtonClicked()
{
	// To be overridden by child classes
}

void UOptionPanelBase::HandleResetButtonClicked()
{
	// To be overridden by child classes
}
