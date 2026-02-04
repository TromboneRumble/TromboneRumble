// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenu/MainMenuWidget.h"
#include "CommonButtonBase.h"
#include "EasyFriendSubsystem.h"
#include "EasySessionSettings.h"
#include "EasySessionSubsystem.h"
#include "EasySessionUtils.h"
#include "OnlineSubsystemUtils.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/EditableText.h"
#include "Components/VerticalBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "UI/UserWidgets/Popup/ConfirmationDialogueWidget.h"
#include "Utilities/DebugHelper.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	const FString MatchMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MatchMenu_Main);
	checkf(!MatchMenuMapPath.IsEmpty(), TEXT("Match menu map path not found. Please set it in GameMapDeveloperSettings."));
	CachedMatchMenuMapPath = MatchMenuMapPath;
}

void UMainMenuWidget::Init()
{
	Super::Init();
	
	if (CB_Online)
	{
		CB_Online->OnClicked().RemoveAll(this);
		CB_Online->OnClicked().AddUObject(this, &ThisClass::HandleOnlineButtonClicked);
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
	CB_Online->SetIsEnabled(bEnabled);
	CB_Join->SetIsEnabled(bEnabled);
	CB_Settings->SetIsEnabled(bEnabled);
	CB_Guide->SetIsEnabled(bEnabled);
	CB_Quit->SetIsEnabled(bEnabled);
}

void UMainMenuWidget::BindSubsystemCallbacks()
{
	Super::BindSubsystemCallbacks();
	
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnStartSessionSuccess.AddUObject(this, &ThisClass::OnStartSessionSuccess);
		SessionsSubsystem->OnStartSessionFailure.AddUObject(this, &ThisClass::OnStartSessionFailure);
		
		SessionsSubsystem->OnFindSessionsSuccess.AddUObject(this, &ThisClass::OnFindSessionsSuccess);
		SessionsSubsystem->OnFindSessionsFailure.AddUObject(this, &ThisClass::OnFindSessionsFailure);
			
		SessionsSubsystem->OnJoinSessionSuccess.AddUObject(this, &ThisClass::OnJoinSessionSuccess);
		SessionsSubsystem->OnJoinSessionFailure.AddUObject(this, &ThisClass::OnJoinSessionFailure);
		
		SessionsSubsystem->OnDestroySessionSuccess.AddUObject(this, &ThisClass::OnDestroySessionSuccess);
		SessionsSubsystem->OnDestroySessionFailure.AddUObject(this, &ThisClass::OnDestroySessionFailure);
	}
	
	if (FriendsSubsystem)
	{
		FriendsSubsystem->SessionInviteAcceptedCustomDelegate.AddLambda([this]() { ShowLoadingOverlay(); });
	}
}

void UMainMenuWidget::RemoveSubsystemCallbacks()
{
	Super::RemoveSubsystemCallbacks();
	
	if (!SessionsSubsystem) return;
	
	SessionsSubsystem->OnStartSessionSuccess.RemoveAll(this);
	SessionsSubsystem->OnStartSessionFailure.RemoveAll(this);
	
	SessionsSubsystem->OnFindSessionsSuccess.RemoveAll(this);
	SessionsSubsystem->OnFindSessionsFailure.RemoveAll(this);
		
	SessionsSubsystem->OnJoinSessionSuccess.RemoveAll(this);
	SessionsSubsystem->OnJoinSessionFailure.RemoveAll(this);
	
	SessionsSubsystem->OnDestroySessionSuccess.RemoveAll(this);
	SessionsSubsystem->OnDestroySessionFailure.RemoveAll(this);
}

void UMainMenuWidget::HandleOnlineButtonClicked()
{
	FString LobbyCode = GenerateRandomLobbyCode(FMath::Max(2, 5));
	
	if (SessionsSubsystem)
	{
		SetUIEnabled(false);
		StartHostValidation(LobbyCode);
		
		ShowLoadingOverlay();
	}
}

void UMainMenuWidget::HandleJoinButtonClicked()
{
	if (ET_Code->GetText().IsEmpty())
	{
		ShowNoticePopup(TEXT("로비 코드를 입력해주세요."));
		return;
	}
	
	if (SessionsSubsystem)
	{
		SetUIEnabled(false);

		FEasySearchSettings SearchSettings;
		SearchSettings.QuerySettings.Add(GKey_Lobby_Code.ToString(), ET_Code->GetText().ToString().ToUpper());
		SessionsSubsystem->FindSessions(SearchSettings);
		
		ShowLoadingOverlay();
	}
}

void UMainMenuWidget::HandleQuitButtonClicked()
{
	if (!CachedQuitDialog)
	{
		CachedQuitDialog = CreateWidget<UConfirmationDialogueWidget>(GetOwningPlayer(), ConfirmationDialogueWidgetClass);
	}
	// TODO : 메세지 관리, change to popup
	const FText Message = FText::FromString(TEXT("정말 게임을 나가실건가요?"));
	CachedQuitDialog->ShowDialogue(Message);
}

void UMainMenuWidget::OnStartSessionSuccess()
{
	HideLoadingOverlay();
	
	const FString MatchMenuPkg = FPackageName::ObjectPathToPackageName(CachedMatchMenuMapPath);
	const FString URL = MatchMenuPkg + TEXT("?listen");
	UGameplayStatics::OpenLevel(this, FName(*URL), true);
}

void UMainMenuWidget::OnStartSessionFailure()
{
	HideLoadingOverlay();
	
	SetUIEnabled(true);
	ShowNoticePopup(TEXT("세션 생성에 실패했습니다. 다시 시도해주세요."));
}

void UMainMenuWidget::OnFindSessionsSuccess(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	if (bIsSearchingForHostValidation)
	{
		bIsSearchingForHostValidation = false;
		
		for (auto Result : SessionResults)
		{
			FString SettingsValue;
			Result.Session.SessionSettings.Get(GKey_Lobby_Code, SettingsValue);
	
			if (SettingsValue == PendingLobbyCode)
			{
				const FString NewCode = GenerateRandomLobbyCode(FMath::Max(2, 5));
				StartHostValidation(NewCode);
				return;
			}
		}
		
		CreateSessionAfterValidation(PendingLobbyCode);
		return;
	}
	
	const FString& LobbyCode = ET_Code->GetText().ToString().ToUpper();

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(GKey_Lobby_Code, SettingsValue);
		
		if (SettingsValue == LobbyCode)
		{
			Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
			Result.Session.SessionSettings.bUsesPresence = true;
			SessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	
	HideLoadingOverlay();
	
	SetUIEnabled(true);
	ShowNoticePopup(FString::Printf(TEXT("'%s'에 해당하는 세션을 찾을 수 없습니다."), *LobbyCode));
}

void UMainMenuWidget::OnFindSessionsFailure(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	if (bIsSearchingForHostValidation)
	{
		bIsSearchingForHostValidation = false;
		SetUIEnabled(true);
		ShowNoticePopup(TEXT("네트워크 상태가 불안정하여 중복 검사에 실패했습니다."));
		HideLoadingOverlay();
		return;
	}
	
	SetUIEnabled(true);
	ShowNoticePopup(TEXT("세션 검색에 실패했습니다. 다시 시도해주세요."));
	HideLoadingOverlay();
}

void UMainMenuWidget::OnJoinSessionSuccess()
{
}

void UMainMenuWidget::OnJoinSessionFailure()
{
	SetUIEnabled(true);
	ShowNoticePopup(TEXT("세션 참가에 실패했습니다. 다시 시도해주세요."));
	HideLoadingOverlay();
}

void UMainMenuWidget::OnDestroySessionSuccess()
{
	PRINT_WITH_CURRENT_CONTEXT("Session destroyed successfully");
}

void UMainMenuWidget::OnDestroySessionFailure()
{
	ShowNoticePopup(TEXT("세션 종료에 실패했습니다. 다시 시도해주세요."));
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
	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("로비 코드 %s가 생성되어 클립보드에 복사되었습니다."), *RandomCode));
	
	return RandomCode;
}

void UMainMenuWidget::StartHostValidation(const FString& Code)
{
	bIsSearchingForHostValidation = true;
	PendingLobbyCode = Code;
    
	FEasySearchSettings SearchSettings;
	SearchSettings.QuerySettings.Add(GKey_Lobby_Code.ToString(), Code); 
	SessionsSubsystem->FindSessions(SearchSettings);
}

void UMainMenuWidget::CreateSessionAfterValidation(const FString& ValidatedCode)
{
	if (SessionsSubsystem)
	{
		FEasySessionSettings Settings;
		Settings.NumPublicConnections = 4;
		Settings.CustomProperties.Add(GKey_Lobby_Code.ToString(), ValidatedCode);
		SessionsSubsystem->CreateSession(Settings);
	}
}