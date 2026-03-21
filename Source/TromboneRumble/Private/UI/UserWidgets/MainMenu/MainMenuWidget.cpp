// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenu/MainMenuWidget.h"
#include "CommonButtonBase.h"
#include "EasyFriendSubsystem.h"
#include "EasyMatchmakingManager.h"
#include "EasyMatchmakingPolicy.h"
#include "EasySessionSettings.h"
#include "EasySessionSubsystem.h"
#include "EasySessionUtils.h"
#include "OnlineSubsystemUtils.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/EditableText.h"
#include "Components/VerticalBox.h"
#include "Framework/TromboneGameInstance.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "UI/UserWidgets/Popup/ConfirmationDialogueWidget.h"
#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "Utilities/TromboneStatics.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	const FString MatchMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MatchMenu_Main);
	checkf(!MatchMenuMapPath.IsEmpty(), TEXT("Match menu map path not found. Please set it in GameMapDeveloperSettings."));
	CachedMatchMenuMapPath = MatchMenuMapPath;
}

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);
	MatchmakingManager->OnMatchmakingUpdated().AddDynamic(this, &ThisClass::HandleMatchmakingUpdated);
}

void UMainMenuWidget::Init()
{
	Super::Init();
	
	if (CB_CreateSession)
	{
		CB_CreateSession->OnClicked().RemoveAll(this);
		CB_CreateSession->OnClicked().AddUObject(this, &ThisClass::HandleCreateSessionClicked);
	}
	if (CB_QuickJoin)
	{
		CB_QuickJoin->OnClicked().RemoveAll(this);
		CB_QuickJoin->OnClicked().AddUObject(this, &ThisClass::HandleQuickJoinButtonClicked);
	}
	if (CB_Join)
	{
		CB_Join->OnClicked().RemoveAll(this);
		CB_Join->OnClicked().AddUObject(this, &ThisClass::HandleJoinButtonClicked);
	}
	if (CB_Settings)
	{
		CB_Settings->OnClicked().RemoveAll(this);
		CB_Settings->OnClicked().AddLambda([this] { SwitchMenu(EMainMenuType::Settings); });
	}
	if (CB_Tutorial)
	{
		CB_Tutorial->OnClicked().RemoveAll(this);
		CB_Tutorial->OnClicked().AddLambda([this] { UTromboneStatics::OpenLevel(GetWorld(), ELevelState::Tutorial); });
	}
	if (CB_Quit)
	{
		CB_Quit->OnClicked().RemoveAll(this);
		CB_Quit->OnClicked().AddUObject(this, &ThisClass::HandleQuitButtonClicked);
	}
	
	if (const IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld()))
	{
		const IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			if (Sessions->GetNamedSession(NAME_GameSession))
			{
				Sessions->DestroySession(NAME_GameSession);
			}
		}
	}
}

void UMainMenuWidget::SetUIEnabled(const bool bEnabled)
{
	Super::SetUIEnabled(bEnabled);
	
	ET_Code->SetIsEnabled(bEnabled);
	CB_QuickJoin->SetIsEnabled(bEnabled);
	CB_Join->SetIsEnabled(bEnabled);
	CB_Settings->SetIsEnabled(bEnabled);
	CB_Tutorial->SetIsEnabled(bEnabled);
	CB_Quit->SetIsEnabled(bEnabled);
	CB_CreateSession->SetIsEnabled(bEnabled);
}

void UMainMenuWidget::BindSubsystemCallbacks()
{
	Super::BindSubsystemCallbacks();
	
	if (SessionsSubsystem)
	{
	}
	
	if (FriendsSubsystem)
	{
		FriendsSubsystem->SessionInviteAcceptedCustomDelegate.AddUObject(this, &ThisClass::ShowLoadingOverlay);
	}
}

void UMainMenuWidget::RemoveSubsystemCallbacks()
{
	Super::RemoveSubsystemCallbacks();
	
	if (SessionsSubsystem)
	{
	}
	
	if (FriendsSubsystem)
	{
		FriendsSubsystem->SessionInviteAcceptedCustomDelegate.RemoveAll(this);
	}
}

void UMainMenuWidget::HandleCreateSessionClicked()
{
	if (USaveManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
	{
		if (Subsystem->ShouldShowTutorialPopup())
		{
			ShowTutorialPopup();
			Subsystem->MarkTutorialAsCompleted();
			return;
		}
	}
	
	FString LobbyCode = GenerateRandomLobbyCode(5);

	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);

	MatchmakingManager->CreateMatchmakingPolicy(FOnCreateMatchmakingPolicyComplete::CreateLambda([this, LobbyCode](UEasyMatchmakingPolicy* MatchmakingPolicy)
	{
		FEasyHostParams HostParams = FEasyHostParams();
		HostParams.StartingLevel = TEXT("/Game/Levels/MatchMenuMap");
		HostParams.bHidden = true;
		HostParams.ExtraSessionSettings.Add(FEasySessionSetting(GKey_Lobby_Code, LobbyCode, EOnlineDataAdvertisementType::ViaOnlineService));
    
		FEasyMatchmakingParams Param = FEasyMatchmakingParams(HostParams);
		int32 Flag = 0;
		Flag |= static_cast<int32>(EEasyMatchmakingFlags::SkipEloChecks);
		
		EEasyMatchmakingMode Mode = EEasyMatchmakingMode::CreateOnly;
    
		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, Flag, Mode);
	}));
}

void UMainMenuWidget::HandleQuickJoinButtonClicked()
{
	if (USaveManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
	{
		if (Subsystem->ShouldShowTutorialPopup())
		{
			ShowTutorialPopup();
			Subsystem->MarkTutorialAsCompleted();
			return;
		}
	}
	
	FString LobbyCode = GenerateRandomLobbyCode(5);
	
	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);
			
	MatchmakingManager->CreateMatchmakingPolicy(FOnCreateMatchmakingPolicyComplete::CreateLambda([this, LobbyCode](UEasyMatchmakingPolicy* MatchmakingPolicy)
	{
		FEasyHostParams HostParams = FEasyHostParams();
		HostParams.StartingLevel = TEXT("/Game/Levels/MatchMenuMap");
		HostParams.bHidden = false;
		HostParams.ExtraSessionSettings.Add(FEasySessionSetting(GKey_Lobby_Code, LobbyCode, EOnlineDataAdvertisementType::ViaOnlineService));
		
		FEasyMatchmakingParams Param = FEasyMatchmakingParams();
		Param.HostParams = HostParams;
		Param.MinSlotsRequired = UEasyStatics::GetPartySize(GetWorld());
												
		int32 Flag = 0;
		Flag |= static_cast<int32>(EEasyMatchmakingFlags::SkipEloChecks);

		const EEasyMatchmakingMode Mode = EEasyMatchmakingMode::Default;
				
		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, Flag, Mode);
	}));
}

void UMainMenuWidget::HandleJoinButtonClicked()
{
	if (ET_Code->GetText().IsEmpty())
	{
		if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
		{
			const FText Message = GI->GetUIText(TEXT("Common_EnterLobbyCode"));
			UNoticePopupWidget* NoticePopup = UTromboneStatics::ShowNoticePopup(GetWorld());
			NoticePopup->OnInit(Message);
		}
		return;
	}
	
	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);
			
	MatchmakingManager->CreateMatchmakingPolicy(FOnCreateMatchmakingPolicyComplete::CreateLambda([this](UEasyMatchmakingPolicy* MatchmakingPolicy)
	{
		const FString LobbyCode = ET_Code->GetText().ToString().ToUpper();
		FEasyMatchmakingParams Param = FEasyMatchmakingParams();
		Param.MinSlotsRequired = UEasyStatics::GetPartySize(GetWorld());
		Param.ExtraQuerySettings.Add(FEasyQuerySetting(GKey_Lobby_Code, LobbyCode, EOnlineComparisonOp::Equals));
												
		int32 Flag = 0;
		Flag |= static_cast<int32>(EEasyMatchmakingFlags::NoHost);
		Flag |= static_cast<int32>(EEasyMatchmakingFlags::SkipEloChecks);
	
		const EEasyMatchmakingMode Mode = EEasyMatchmakingMode::Default;
				
		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, Flag, Mode);
	}));
}

void UMainMenuWidget::HandleQuitButtonClicked()
{
	if (!CachedQuitDialog)
	{
		CachedQuitDialog = CreateWidget<UConfirmationDialogueWidget>(GetOwningPlayer(), ConfirmationDialogueWidgetClass);
	}

	UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance());
	const FText Message = GI ? GI->GetUIText(TEXT("Confirmation_QuitGame")) : FText::FromString(TEXT("Default Quit Message"));
	CachedQuitDialog->ShowDialogue(Message);
}

void UMainMenuWidget::HandleMatchmakingUpdated(const EEasyMatchmakingState MatchmakingState, const int32 MatchmakingTime)
{
	UE_LOG(LogTemp, Log, TEXT("Matchmaking State Updated: %s, Time: %d"), LexToString(MatchmakingState), MatchmakingTime);
	
	if (UEasyStatics::IsMatchmaking(GetWorld()))
	{
		ShowLoadingOverlay();
	}
	else
	{
		HideLoadingOverlay();
	}
	
	if (MatchmakingState == EEasyMatchmakingState::JoiningSession)
	{
		SetUIEnabled(false);
	}
}

FString UMainMenuWidget::GenerateRandomLobbyCode(int32 Length) const
{
	const FString Chars = TEXT("ABCDEFGHJKMNPQRSTUVWXYZ23456789");
	FString RandomCode;
	for (int32 i = 0; i < Length; ++i)
	{
		RandomCode += Chars[FMath::RandRange(0, Chars.Len() - 1)];
	}
	
	FPlatformApplicationMisc::ClipboardCopy(*RandomCode);
	
	return RandomCode;
}

void UMainMenuWidget::ShowTutorialPopup()
{
	if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		const FText Title = GI->GetTutorialUIText(TEXT("StringKey_TutorialFirstPlayerShowPopupTitle"));
		const FText Description = GI->GetTutorialUIText(TEXT("StringKey_TutorialFirstPlayerShowPopupDescription"));
		const FText LeftButtonText = GI->GetUIText(TEXT("Common_No"));
		const FText RightButtonText = GI->GetUIText(TEXT("Common_Yes"));
		
		UTwoButtonWithoutClosePopup* Popup = UTromboneStatics::ShowTwoButtonPopup(GetWorld());
		
		FOnPopupAction LeftAction, RightAction;
		LeftAction.AddLambda([this, Popup]()
		{
			Popup->ClosePopup();
		});
		RightAction.AddLambda([this]()
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelState::Tutorial);
		});
		
		Popup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftAction, RightAction);
	}
}