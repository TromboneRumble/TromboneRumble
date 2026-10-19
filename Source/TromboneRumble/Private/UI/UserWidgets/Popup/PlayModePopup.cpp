// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Popup/PlayModePopup.h"
#include "CommonButtonBase.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "CommonTextBlock.h"
#include "EasySessionStatics.h"
#include "EasySessionSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/EditableText.h"
#include "Data/UIData.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/TromboneGameInstance.h"
#include "Subsystems/AppearanceSubsystem.h"
#include "Subsystems/ToastSubsystem.h"
#include "UI/UserWidgets/Common/CommonButtonBaseExtension.h"
#include "Utilities/TromboneStatics.h"

void UPlayModePopup::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// PlayModePopup UI buttons overlap each other, wire them manually
	if (CB_CreateSession && CB_QuickJoin && CB_JoinCode && CB_Join)
	{
		CB_CreateSession->SetNavigationRuleExplicit(EUINavigation::Left, CB_JoinCode);
		CB_CreateSession->SetNavigationRuleExplicit(EUINavigation::Right, CB_QuickJoin);
		CB_CreateSession->SetNavigationRuleExplicit(EUINavigation::Down, Button_Close);
		
		CB_QuickJoin->SetNavigationRuleExplicit(EUINavigation::Left, CB_CreateSession);
		CB_QuickJoin->SetNavigationRuleExplicit(EUINavigation::Right, CB_JoinCode);
		CB_QuickJoin->SetNavigationRuleExplicit(EUINavigation::Down, Button_Close);
		
		CB_JoinCode->SetNavigationRuleExplicit(EUINavigation::Left, CB_QuickJoin);
		CB_JoinCode->SetNavigationRuleExplicit(EUINavigation::Right, CB_CreateSession);
		CB_JoinCode->SetNavigationRuleExplicit(EUINavigation::Down, CB_Join);

		CB_Join->SetNavigationRuleExplicit(EUINavigation::Up, CB_JoinCode);
		CB_Join->SetNavigationRuleExplicit(EUINavigation::Down, Button_Close);
	}
}

void UPlayModePopup::Register()
{
	Super::Register();

	if (ET_Code)
	{
		ET_Code->SetText(FText::GetEmpty());
	}

	if (CB_CreateSession)
	{
		CB_CreateSession->OnClicked().RemoveAll(this);
		CB_CreateSession->OnClicked().AddUObject(this, &ThisClass::HandleCreateSessionClicked);
	}
	if (CB_QuickJoin)
	{
		CB_QuickJoin->OnClicked().RemoveAll(this);
		CB_QuickJoin->OnClicked().AddUObject(this, &ThisClass::HandleQuickJoinClicked);
	}
	if (CB_JoinCode)
	{
		CB_JoinCode->OnClicked().RemoveAll(this);
		CB_JoinCode->OnClicked().AddUObject(this, &ThisClass::HandleJoinCodeClicked);
	}
	if (CB_Join)
	{
		CB_Join->OnClicked().RemoveAll(this);
		CB_Join->OnClicked().AddUObject(this, &ThisClass::HandleJoinClicked);
	}

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
		HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
	}
}

void UPlayModePopup::Unregister()
{
	Super::Unregister();

	if (CB_CreateSession)
	{
		CB_CreateSession->OnClicked().RemoveAll(this);
	}
	if (CB_QuickJoin)
	{
		CB_QuickJoin->OnClicked().RemoveAll(this);
	}
	if (CB_JoinCode)
	{
		CB_JoinCode->OnClicked().RemoveAll(this);
	}
	if (CB_Join)
	{
		CB_Join->OnClicked().RemoveAll(this);
	}

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
	}
}

UWidget* UPlayModePopup::GetDefaultFocusWidget() const
{
	// ET_Code is skipped on purpose. The text field should not take focus until the player asks for it
	if (UWidget* Candidate = FirstFocusCandidate({ CB_CreateSession, CB_QuickJoin, CB_JoinCode }))
	{
		return Candidate;
	}
	return Super::GetDefaultFocusWidget();
}

void UPlayModePopup::HandleInputMethodChanged(const ECommonInputType NewInputType)
{
	if (!Text_GamepadHint)
	{
		return;
	}

	const bool bGamepad = NewInputType == ECommonInputType::Gamepad;
	Text_GamepadHint->SetVisibility(bGamepad ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UPlayModePopup::HandleCreateSessionClicked()
{
	UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(this);
	if (!Sessions)
	{
		return;
	}

	// A created room is hidden and found only by its code, which EasySession generates with the project's Join Code Length.
	FEasySessionHostParams HostParams;
	HostParams.InitialMapName = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_MatchMenu);
	HostParams.bHidden = true;
	HostParams.bUseJoinCode = true;
	HostParams.CustomSettings.Add(UTromboneStatics::InGameMapSettingKey, UTromboneConfig::Get()->DefaultInGameMap.ToString());

	// Creating a room is not a matchmaking run, so the main menu's matchmaking events do not cover it.
	// The overlay and the failure toast are shown here instead.
	const TWeakObjectPtr<APlayerController> WeakPC = GetOwningPlayer();
	UTromboneStatics::ShowLoadingOverlay(WeakPC.Get());

	Sessions->CreateSession(HostParams, FEasySessionCompleteDelegate::CreateWeakLambda(Sessions, [Sessions, WeakPC](EEasySessionResult Result, const FString& ErrorMessage)
	{
		if (Result == EEasySessionResult::Success)
		{
			// The code goes to the clipboard as soon as the room exists, so the host can paste it to friends.
			UTromboneStatics::CopyRoomCodeToClipboard(Sessions);

			if (UAppearanceSubsystem* AppearanceSubsystem = Sessions->GetGameInstance()->GetSubsystem<UAppearanceSubsystem>())
			{
				AppearanceSubsystem->ResetColors();
			}
			return;
		}

		// TODO : 로컬라이징
		UTromboneStatics::ShowToast(Sessions->GetWorld(), UTromboneStatics::MakeToastRequest(FText::FromString(TEXT("Matchmaking failed."))));
		UTromboneStatics::PopOverlay(WeakPC.Get());
	}));
}

void UPlayModePopup::HandleQuickJoinClicked()
{
	UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(this);
	if (!Sessions)
	{
		return;
	}

	// Joins an open room whose match has not started, or hosts one when there is none.
	// EasySession makes the hosted room public, so the next player's quick join finds it.
	FEasyMatchmakingParams Params;
	Params.Search.MinOpenSlots = 1;
	Params.Search.bIncludeInProgressSessions = false;
	Params.bAllowHostFallback = true;
	Params.Host.InitialMapName = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_MatchMenu);
	Params.Host.bUseJoinCode = true;

	Sessions->StartMatchmaking(Params);
}

void UPlayModePopup::HandleJoinCodeClicked()
{
	// Just move the focus to the code input field
	if (ET_Code)
	{
		ET_Code->SetFocus();
	}
}

void UPlayModePopup::HandleJoinClicked()
{
	if (!ET_Code || ET_Code->GetText().IsEmpty())
	{
		UToastSubsystem* ToastSubsystem = GetGameInstance()->GetSubsystem<UToastSubsystem>();
		const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance());

		if (!ToastSubsystem || !GI)
		{
			return;
		}

		const FText EmptyLobbyCodeWarningText = GI->GetCommonUIText(TEXT("Common_EnterLobbyCode"));
		const FToastRequest Request(EmptyLobbyCodeWarningText);
		ToastSubsystem->ShowToast(Request);

		return;
	}

	UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(this);
	if (!Sessions)
	{
		return;
	}

	// The code search also returns the hidden room it belongs to. EasySession compares the code in upper case.
	FEasyMatchmakingParams Params;
	Params.Search.JoinCode = ET_Code->GetText().ToString();
	Params.Search.MinOpenSlots = 1;
	Params.bAllowHostFallback = false;

	Sessions->StartMatchmaking(Params);
}

