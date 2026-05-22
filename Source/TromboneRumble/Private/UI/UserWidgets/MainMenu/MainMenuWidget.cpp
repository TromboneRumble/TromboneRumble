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
#include "Data/UIData.h"
#include "Framework/TromboneGameInstance.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Subsystems/AppearanceSubsystem.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "Subsystems/ToastSubsystem.h"
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
	
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().AddDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingComplete().AddDynamic(this, &ThisClass::HandleMatchmakingComplete);
		MatchmakingManager->OnMatchmakingCanceled().AddDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}
	
	if (UAppearanceSubsystem* AppearanceSubsystem = GetGameInstance()->GetSubsystem<UAppearanceSubsystem>())
	{
		AppearanceSubsystem->ResetColors();
	}
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
		CB_Settings->OnClicked().AddLambda([this]
		{
			UTromboneStatics::ShowPopup<USettingPopup>(GetWorld());
		});
	}
	if (CB_Tutorial)
	{
		CB_Tutorial->OnClicked().RemoveAll(this);
		CB_Tutorial->OnClicked().AddUObject(this, &ThisClass::HandleTutorialButtonClicked);
	}
	if (CB_Quit)
	{
		CB_Quit->OnClicked().RemoveAll(this);
		CB_Quit->OnClicked().AddLambda([this] { ShowQuitPopup(); });
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
	
	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);
	MatchmakingManager->CreateMatchmakingPolicy(FOnCreateMatchmakingPolicyComplete::CreateLambda([this](UEasyMatchmakingPolicy* MatchmakingPolicy)
	{
		const UTromboneConfig* Config = UTromboneConfig::Get();
		const FString RoomCode = UTromboneStatics::GenerateRandomRoomCode(Config->RoomCodeLength);
		
		FEasyHostParams HostParams = FEasyHostParams();
		HostParams.StartingLevel = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MatchMenu_Main);
		HostParams.bHidden = true;
		HostParams.ExtraSessionSettings.Add(FEasySessionSetting(GKey_Lobby_Code, RoomCode, EOnlineDataAdvertisementType::ViaOnlineService));

		const FEasyMatchmakingParams Param = FEasyMatchmakingParams(HostParams);
		int32 Flag = 0;
		Flag |= static_cast<int32>(EEasyMatchmakingFlags::SkipEloChecks);

		const EEasyMatchmakingMode Mode = EEasyMatchmakingMode::CreateOnly;
    
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
	
	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);
	MatchmakingManager->CreateMatchmakingPolicy(FOnCreateMatchmakingPolicyComplete::CreateLambda([this](UEasyMatchmakingPolicy* MatchmakingPolicy)
	{
		const UTromboneConfig* Config = UTromboneConfig::Get();
		const FString RoomCode = UTromboneStatics::GenerateRandomRoomCode(Config->RoomCodeLength);
		
		FEasyHostParams HostParams = FEasyHostParams();
		HostParams.StartingLevel = TEXT("/Game/Levels/MatchMenuMap");
		HostParams.bHidden = true;
		HostParams.ExtraSessionSettings.Add(FEasySessionSetting(GKey_Lobby_Code, RoomCode, EOnlineDataAdvertisementType::ViaOnlineService));
		
		FEasyMatchmakingParams Param = FEasyMatchmakingParams();
		Param.HostParams = HostParams;
		Param.MinSlotsRequired = 1;
												
		int32 Flag = 0;
		Flag |= static_cast<int32>(EEasyMatchmakingFlags::SkipEloChecks);

		const EEasyMatchmakingMode Mode = EEasyMatchmakingMode::Default;
				
		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, Flag, Mode);
	}));
}

void UMainMenuWidget::HandleJoinButtonClicked()
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
	
		const EEasyMatchmakingMode Mode = EEasyMatchmakingMode::Default;
				
		MatchmakingPolicy->StartMatchmaking(NAME_GameSession, Param, Flag, Mode);
	}));
}

void UMainMenuWidget::HandleTutorialButtonClicked()
{
	if (USaveManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
	{
		Subsystem->MarkTutorialAsCompleted();
	}
	
	UTromboneStatics::OpenLevel(GetWorld(), ELevelState::Tutorial);
}

void UMainMenuWidget::HandleMatchmakingStarted()
{
	SetUIEnabled(false);
}

void UMainMenuWidget::HandleMatchmakingComplete(const FName SessionName, const EEasyMatchmakingCompleteResult Result)
{
	if (Result == EEasyMatchmakingCompleteResult::Failure || Result == EEasyMatchmakingCompleteResult::NoResults)
	{
		SetUIEnabled(true);
	}
}

void UMainMenuWidget::HandleMatchmakingCanceled()
{
	SetUIEnabled(true);
}

void UMainMenuWidget::ShowTutorialPopup()
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		const FText Title = GI->GetTutorialUIText(TEXT("StringKey_TutorialFirstPlayerShowPopupTitle"));
		const FText Description = GI->GetTutorialUIText(TEXT("StringKey_TutorialFirstPlayerShowPopupDescription"));
		const FText LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
		const FText RightButtonText = GI->GetCommonUIText(TEXT("Common_No"));
		
		UTwoButtonWithoutClosePopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonWithoutClosePopup>(GetWorld());

		const TFunction<void()> LeftCallback = [this]()
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelState::Tutorial);
		};

		const TFunction<void()> RightCallback = [this, Popup]()
		{
			Popup->ClosePopup();
		};
		
		
		Popup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftCallback, RightCallback);
	}
}

void UMainMenuWidget::ShowQuitPopup()
{
	if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		const FText Title = FText::FromString(TEXT(""));
		const FText Description = GI->GetCommonUIText(TEXT("Confirmation_QuitGame"));
		const FText LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
		const FText RightButtonText = GI->GetCommonUIText(TEXT("Common_No"));
		
		if (UTwoButtonWithoutClosePopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonWithoutClosePopup>(GetWorld()))
		{
			const TFunction<void()> LeftCallback = [this]()
			{
				APlayerController* PC = GetOwningPlayer();
				UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
			};

			const TFunction<void()> RightCallback = [this, Popup]()
			{
				Popup->ClosePopup();
			};
			
			Popup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftCallback, RightCallback);
		}
	}
}
