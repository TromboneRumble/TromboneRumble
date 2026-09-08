// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Popup/PlayModePopup.h"
#include "CommonButtonBase.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "CommonTextBlock.h"
#include "EasyMatchmakingManager.h"
#include "EasyMatchmakingPolicy.h"
#include "EasySessions.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/EditableText.h"
#include "Data/UIData.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/TromboneGameInstance.h"
#include "Online/OnlineSessionNames.h"
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
	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);
	MatchmakingManager->CreateMatchmakingPolicy(FOnCreateMatchmakingPolicyComplete::CreateLambda([this](UEasyMatchmakingPolicy* MatchmakingPolicy)
	{
		const UTromboneConfig* Config = UTromboneConfig::Get();
		const FString RoomCode = UTromboneStatics::GenerateRandomRoomCode(Config->RoomCodeLength);

		FEasyHostParams HostParams = FEasyHostParams();
		HostParams.StartingLevel = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_MatchMenu);
		HostParams.bHidden = true;
		HostParams.ExtraSessionSettings.Add(FEasySessionSetting(SETTING_LOBBYCODE, RoomCode, EOnlineDataAdvertisementType::ViaOnlineService));
		HostParams.ExtraSessionSettings.Add(FEasySessionSetting(SETTING_MAPNAME, Config->DefaultInGameMap.ToString(), EOnlineDataAdvertisementType::ViaOnlineService));

		const FEasyMatchmakingParams Param = FEasyMatchmakingParams(HostParams);

		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, 0, EEasyMatchmakingMode::CreateOnly);
	}));
}

void UPlayModePopup::HandleQuickJoinClicked()
{
	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);
	MatchmakingManager->CreateMatchmakingPolicy(FOnCreateMatchmakingPolicyComplete::CreateLambda([this](UEasyMatchmakingPolicy* MatchmakingPolicy)
	{
		const UTromboneConfig* Config = UTromboneConfig::Get();
		const FString RoomCode = UTromboneStatics::GenerateRandomRoomCode(Config->RoomCodeLength);

		FEasyHostParams HostParams = FEasyHostParams();
		HostParams.StartingLevel = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_MatchMenu);
		HostParams.bHidden = true;
		HostParams.ExtraSessionSettings.Add(FEasySessionSetting(SETTING_LOBBYCODE, RoomCode, EOnlineDataAdvertisementType::ViaOnlineService));

		FEasyMatchmakingParams Param = FEasyMatchmakingParams();
		Param.HostParams = HostParams;
		Param.MinSlotsRequired = 1;

		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, 0, EEasyMatchmakingMode::Default);
	}));
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

	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);

	MatchmakingManager->CreateMatchmakingPolicy(FOnCreateMatchmakingPolicyComplete::CreateLambda([this](UEasyMatchmakingPolicy* MatchmakingPolicy)
	{
		const FString LobbyCode = ET_Code->GetText().ToString().ToUpper();
		FEasyMatchmakingParams Param = FEasyMatchmakingParams();
		Param.MinSlotsRequired = 1;
		Param.ExtraQuerySettings.Add(FEasyQuerySetting(SETTING_LOBBYCODE, LobbyCode, EOnlineComparisonOp::Equals));

		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, static_cast<int32>(EEasyMatchmakingFlags::NoHost), EEasyMatchmakingMode::Default);
	}));
}

