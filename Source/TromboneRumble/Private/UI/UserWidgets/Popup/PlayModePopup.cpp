// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Popup/PlayModePopup.h"
#include "CommonButtonBase.h"
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
#include "Utilities/TromboneStatics.h"

void UPlayModePopup::Register()
{
	Super::Register();

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
}

UWidget* UPlayModePopup::NativeGetDesiredFocusTarget() const
{
	return CB_CreateSession;
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

