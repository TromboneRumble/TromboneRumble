#include "UI/UserWidgets/Popup/EscapePopup.h"
#include "EasyOnlineSession.h"
#include "Actors/ResetCollider.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/ActorComponents/ClientToServerRelayComponent.h"
#include "UI/UserWidgets/Common/CommonButtonBaseExtensionWithText.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/TromboneStatics.h"

UWidget* UEscapePopup::GetDefaultFocusWidget() const
{
	if (UWidget* Candidate = FirstFocusCandidate({ Button_Option, Button_Disconnect }))
	{
		return Candidate;
	}
	return Super::GetDefaultFocusWidget();
}

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
	if (Button_Teleport)
	{
		Button_Teleport->OnClicked().RemoveAll(this);
		Button_Teleport->OnClicked().AddUObject(this, &ThisClass::HandleTeleportButtonClicked);
		
		const bool bHasResetCollider = (AResetCollider::FindInLevel(this) != nullptr);
		Button_Teleport->SetVisibility(bHasResetCollider ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UEscapePopup::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 확인 팝업이 닫히면서 되살아난 경우, 곧바로 자신도 닫아 게임 화면으로 돌아간다
	if (bCloseOnNextActivation)
	{
		bCloseOnNextActivation = false;
		ClosePopup(true);
	}
}

void UEscapePopup::Unregister()
{
	Super::Unregister();

	if (Button_Option)
	{
		Button_Option->OnClicked().RemoveAll(this);
	}
	if (Button_Disconnect)
	{
		Button_Disconnect->OnClicked().RemoveAll(this);
	}
	if (Button_Teleport)
	{
		Button_Teleport->OnClicked().RemoveAll(this);
	}
}

void UEscapePopup::HandleOptionButtonClicked() const
{
	const USettingPopup* Popup = UTromboneStatics::ShowPopup<USettingPopup>(GetWorld());
	if (!Popup)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to show setting popup"));
		return;
	}
}

void UEscapePopup::HandleDisconnectButtonClicked() const
{
	if (UTwoButtonPopup* ConfirmPopup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
	{
		FTwoButtonPopupParams Params;
		Params.Title = ConfirmTitle;
		Params.Content = ConfirmDescription;
		Params.LeftButtonText = ConfirmLeftButton;
		Params.RightButtonText = ConfirmRightButton;
	
		Params.LeftCallback = [this]()
		{
			if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
			{
				OnlineSession->LeaveGameSession();
			}
		};

		ConfirmPopup->Init(Params);
	}
}

void UEscapePopup::HandleTeleportButtonClicked()
{
	if (UTwoButtonPopup* ConfirmPopup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
	{
		FTwoButtonPopupParams Params;
		Params.Title = TeleportConfirmTitle;
		Params.Content = TeleportConfirmDescription;
		Params.LeftButtonText = ConfirmLeftButton;
		Params.RightButtonText = ConfirmRightButton;
		
		Params.LeftCallback = [this]()
		{
			RequestTeleportToResetPoint();
		};

		ConfirmPopup->Init(Params);
	}
}

void UEscapePopup::RequestTeleportToResetPoint()
{
	AResetCollider* ResetCollider = AResetCollider::FindInLevel(this);
	if (!ResetCollider)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("No ResetCollider found in level"));
		return;
	}

	const APlayerController* PlayerController = GetOwningPlayer();
	ADefaultTromboneCharacter* LocalCharacter = Cast<ADefaultTromboneCharacter>(PlayerController ? PlayerController->GetPawn() : nullptr);
	if (!LocalCharacter)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("No local character to teleport"));
		return;
	}

	if (LocalCharacter->HasAuthority())
	{
		// 리슨 서버 호스트는 서버 로직을 직접 실행
		ResetCollider->HandleServerRPC(LocalCharacter);
	}
	else
	{
		// 클라이언트는 Owner 없는 월드 액터에 Server RPC를 직접 호출할 수 없으므로 relay 경유
		if (UClientToServerRelayComponent* Relay = LocalCharacter->GetClientToServerRelayComponent())
		{
			Relay->Server_SendRPCRequest(ResetCollider);
		}
	}

	// 비상탈출 후 게임 화면으로 복귀.
	if (IsActivated())
	{
		ClosePopup(true);
	}
	else
	{
		bCloseOnNextActivation = true;
	}
}
