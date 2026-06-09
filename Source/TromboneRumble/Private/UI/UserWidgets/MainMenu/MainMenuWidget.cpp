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
#include "Framework/TromboneGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Subsystems/AppearanceSubsystem.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Popup/TwoButtonPopup.h"
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
	if (CB_Customize)
	{
		CB_Customize->OnClicked().RemoveAll(this);
		CB_Customize->OnClicked().AddUObject(this, &ThisClass::HandleCustomizeButtonClicked);
	}
	if (CB_Tutorial)
	{
		CB_Tutorial->OnClicked().RemoveAll(this);
		CB_Tutorial->OnClicked().AddUObject(this, &ThisClass::HandleTutorialButtonClicked);
	}
	if (CB_Quit)
	{
		CB_Quit->OnClicked().RemoveAll(this);
		CB_Quit->OnClicked().AddUObject(this, &ThisClass::ShowQuitPopup);
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
	
	UTromboneStatics::ShowPopup<UJoinCodePopup>(GetWorld());
}

void UMainMenuWidget::HandleCustomizeButtonClicked()
{
	UTromboneStatics::OpenLevel(GetWorld(), ELevelType::Customize);
}

void UMainMenuWidget::HandleTutorialButtonClicked()
{
	if (USaveManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
	{
		Subsystem->MarkTutorialAsCompleted();
	}
	
	UTromboneStatics::OpenLevel(GetWorld(), ELevelType::Tutorial);
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
		FTwoButtonPopupParams Params;
		Params.Title = GI->GetTutorialUIText(TEXT("StringKey_TutorialFirstPlayerShowPopupTitle"));
		Params.Content = GI->GetTutorialUIText(TEXT("StringKey_TutorialFirstPlayerShowPopupDescription"));
		Params.LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
		Params.RightButtonText = GI->GetCommonUIText(TEXT("Common_No"));
		
		Params.LeftCallback = [this]
		{ 
			UTromboneStatics::OpenLevel(GetWorld(), ELevelType::Tutorial);
		};
		
		if (UTwoButtonPopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
		{
			Popup->Init(Params);
		}
	}
}

void UMainMenuWidget::ShowQuitPopup() const
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		FTwoButtonPopupParams Params;
		Params.Title = FText::GetEmpty();
		Params.Content = GI->GetCommonUIText(TEXT("Confirmation_QuitGame"));
		Params.LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
		Params.RightButtonText = GI->GetCommonUIText(TEXT("Common_No"));
    
		Params.LeftCallback = [this]()
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
			}
		};
		
		if (UTwoButtonPopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
		{
			Popup->Init(Params);
		}
	}
}
