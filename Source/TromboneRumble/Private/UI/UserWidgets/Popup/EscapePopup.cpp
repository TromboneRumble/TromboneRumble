#include "UI/UserWidgets/Popup/EscapePopup.h"
#include "EasyOnlineSession.h"
#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/TromboneStatics.h"

void UEscapePopup::Register()
{
	Super::Register();
	
	if (Button_Option)
	{
		Button_Option->OnClicked().RemoveAll(this);
		Button_Option->OnClicked().AddUObject(this, &ThisClass::HandleOptionButtonClicked);
	}
	if (Button_Disconnect)
	{
		Button_Disconnect->OnClicked().RemoveAll(this);
		Button_Disconnect->OnClicked().AddUObject(this, &ThisClass::HandleDisconnectButtonClicked);
	}
}

void UEscapePopup::HandleOptionButtonClicked()
{
	const USettingPopup* Popup = UTromboneStatics::ShowPopup<USettingPopup>(GetWorld());
	if (!Popup)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to show setting popup"));
		return;
	}
}

void UEscapePopup::HandleDisconnectButtonClicked()
{
	if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
	{
		OnlineSession->LeaveGameSession();
		return;
	}
	
	LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to leave game session"));
}
