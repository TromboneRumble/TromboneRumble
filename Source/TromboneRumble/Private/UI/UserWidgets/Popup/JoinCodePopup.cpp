// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Popup/JoinCodePopup.h"
#include "CommonButtonBase.h"
#include "EasyMatchmakingManager.h"
#include "EasyMatchmakingPolicy.h"
#include "EasySessionSettings.h"
#include "Components/EditableText.h"
#include "Data/UIData.h"
#include "Framework/TromboneGameInstance.h"
#include "Subsystems/ToastSubsystem.h"

void UJoinCodePopup::Register()
{
	Super::Register();
	
	if (Button_JoinCode)
	{
		Button_JoinCode->OnClicked().RemoveAll(this);
		Button_JoinCode->OnClicked().AddUObject(this, &UJoinCodePopup::OnClickJoinCode);
	}
}

void UJoinCodePopup::Unregister()
{
	Super::Unregister();
	
	if (Button_JoinCode)
	{
		Button_JoinCode->OnClicked().RemoveAll(this);
	}
}

void UJoinCodePopup::OnClickJoinCode()
{
	if (ET_Code->GetText().IsEmpty())
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
		Param.ExtraQuerySettings.Add(FEasyQuerySetting(GKey_Lobby_Code, LobbyCode, EOnlineComparisonOp::Equals));
												
		int32 Flag = 0;
		Flag |= static_cast<int32>(EEasyMatchmakingFlags::NoHost);
		Flag |= static_cast<int32>(EEasyMatchmakingFlags::SkipEloChecks);

		constexpr EEasyMatchmakingMode Mode = EEasyMatchmakingMode::Default;
				
		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, Flag, Mode);
	}));
	
	ClosePopup();
}
