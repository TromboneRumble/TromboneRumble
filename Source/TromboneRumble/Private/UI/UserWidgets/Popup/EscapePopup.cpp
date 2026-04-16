#include "UI/UserWidgets/Popup/EscapePopup.h"
#include "EasyOnlineSession.h"
#include "Framework/TromboneGameInstance.h"
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
	UTwoButtonWithoutClosePopup* ConfirmPopup = UTromboneStatics::ShowPopup<UTwoButtonWithoutClosePopup>(GetWorld());
	if (!ConfirmPopup)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to show disconnect confirmation popup"));
		return;
	}
	
	if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		// TODO : UI 용 String 테이블 따로 만들어서 관리하기
		const FText Title = FText::FromString(TEXT("일시정지"));
		const FText Description = FText::FromString(TEXT("정말로 게임에서 나가시겠습니까?\n현재 매치의 진행 상황과 기록이\n모두 초기화 됩니다."));
		const FText LeftButtonText = FText::FromString(TEXT("계속하기"));
		const FText RightButtonText = FText::FromString(TEXT("나가기"));
		
		const TFunction<void()> LeftCallback = [this, ConfirmPopup]()
		{
			ConfirmPopup->ClosePopup();
		};
		const TFunction<void()> RightCallback = [this]()
		{
			if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
			{
				OnlineSession->LeaveGameSession();
			}
		};
		
		ConfirmPopup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftCallback, RightCallback);
	}
	
	LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to leave game session"));
}
