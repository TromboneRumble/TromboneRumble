// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenu/MatchMenuWidget.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "EasyExternalUILibrary.h"
#include "EasySessionSettings.h"
#include "EasySessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Framework/TromboneGameInstance.h"
#include "Framework/GameState/MatchMenuGameState.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameStateSubsystem.h"
#include "UI/UserWidgets/Common/CommonRotatorWidgetBase.h"

void UMatchMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	BindGameStateEvents();
	
	const FString MainMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
	checkf(!MainMenuMapPath.IsEmpty(), TEXT("Main menu map path not found. Please set it in GameMapDeveloperSettings."));
	CachedMainMenuMapPath = MainMenuMapPath;
	
	const FString LobbyMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);
	checkf(!LobbyMapPath.IsEmpty(), TEXT("Lobby map path not found. Please set it in GameMapDeveloperSettings."));
	CachedLobbyMapPath = LobbyMapPath;
}

void UMatchMenuWidget::NativeDestruct()
{
	RemoveSubsystemCallbacks();
	RemoveGameStateEvents();

	Super::NativeDestruct();
}

void UMatchMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem) return;

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
    
	if (CurrentSession && CurrentSession->SessionSettings.Settings.Num() > 0)
	{
		if (const FOnlineSessionSetting* Setting = CurrentSession->SessionSettings.Settings.Find(GKey_Lobby_Code))
		{
			if (CT_Code)
			{
				const FString Prefix = TEXT("입장 코드 : ");
				FString OutCode;
				Setting->Data.GetValue(OutCode);
				CT_Code->SetText(FText::FromString(Prefix + OutCode));
			}
		}
	}
}

void UMatchMenuWidget::Init()
{
	Super::Init();
	
	const APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	const bool bIsClient = !PC->HasAuthority();
	
	if (CB_Start)
	{
		CB_Start->OnClicked().RemoveAll(this);
		CB_Start->OnClicked().AddUObject(this, &ThisClass::HandleStartButtonClicked);
		if (bIsClient)
		{
			CB_Start->SetIsEnabled(false);
		}
	}
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddUObject(this, &ThisClass::HandleBackButtonClicked);
	}
	if (CB_Invite)
	{
		CB_Invite->OnClicked().RemoveAll(this);
		CB_Invite->OnClicked().AddUObject(this, &ThisClass::HandleInviteButtonClicked);
	}
	if (CR_MatchType)
	{
		CR_MatchType->OnRotatedWithDirection().RemoveAll(this);
		CR_MatchType->OnRotatedWithDirection().AddDynamic(this, &ThisClass::HandleOnRotatedMatchType);
		if (bIsClient)
		{
			CR_MatchType->SetIsEnabled(false);
		}
	}
}

void UMatchMenuWidget::BindGameStateEvents()
{
	RemoveGameStateEvents();
	
	if (AMatchMenuGameState* MatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGS->OnPlayerListChanged.AddDynamic(this, &ThisClass::OnPlayerListChanged);
		OnPlayerListChanged(MatchMenuGS->GetPlayerList());
		
		MatchMenuGS->OnMatchTypeChanged.AddDynamic(this, &ThisClass::OnMatchTypeChanged);
		OnMatchTypeChanged(MatchMenuGS->GetCurrentMatchType());
	}
}

void UMatchMenuWidget::RemoveGameStateEvents()
{
	if (AMatchMenuGameState* MatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGS->OnPlayerListChanged.RemoveAll(this);
		MatchMenuGS->OnMatchTypeChanged.RemoveAll(this);
	}
}

void UMatchMenuWidget::OnPlayerListChanged(const TArray<FString>& PlayerNames)
{
	if (!CT_PlayerList || bIsStarted) return;

	FString FormattedPlayerList;

	for (int32 i = 0; i < PlayerNames.Num(); ++i)
	{
		FormattedPlayerList.Append(FString::Printf(TEXT("%d. %s\n"), i + 1, *PlayerNames[i]));
	}
	
	CT_PlayerList->SetText(FText::FromString(FormattedPlayerList));
}

void UMatchMenuWidget::OnMatchTypeChanged(EMatchType NewType)
{
	if (CR_MatchType)
	{
		const int32 Index = static_cast<int32>(NewType);
		CR_MatchType->SetSelectedIndex(Index);
	}
}

void UMatchMenuWidget::HandleStartButtonClicked()
{
	bIsStarted = true;
	SetUIEnabled(false);

	if (AMatchMenuGameState* MatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGS->SetIsTransitioningToInGame(true);
	}
	
	if (UTromboneGameInstance* TromboneGI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		if (const UGameStateSubsystem* GameStateSubsystem = TromboneGI->GetSubsystem<UGameStateSubsystem>())
		{
			if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
			{
				const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
				if (SessionInterface.IsValid())
				{
					if (const FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession))
					{
						TromboneGI->SetSessionPlayerNumber(Session->RegisteredPlayers.Num());
					}
				}
			}
			
			const FString MapPath = GameStateSubsystem->GetMapNameForTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);

			UWorld* World = GetWorld();
			if (!World || World->GetAuthGameMode() == nullptr || MapPath.IsEmpty()) return;
			
			if (!World->ServerTravel(MapPath))
			{
				bIsStarted = false;
				SetUIEnabled(true);
				ShowNoticePopup(TEXT("게임 시작에 실패하였습니다."));
			}
		}
	}
}

void UMatchMenuWidget::HandleBackButtonClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEasySessionSubsystem* EasySessionSubsystem = GI->GetSubsystem<UEasySessionSubsystem>())
		{
			EasySessionSubsystem->DestroySession();
		}
	}
	const FString MainMenuPkg = FPackageName::ObjectPathToPackageName(CachedMainMenuMapPath);
	const FString URL = MainMenuPkg;
	UGameplayStatics::OpenLevel(this, FName(*URL), true);
}

void UMatchMenuWidget::HandleInviteButtonClicked()
{
	EEasyResultType OutResult;
	UEasyExternalUILibrary::ShowInviteUI(GetOwningPlayer(), OutResult);
}

void UMatchMenuWidget::HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir)
{
	if (AMatchMenuGameState* MatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGS->SetMatchType(static_cast<EMatchType>(Value));
	}
}

void UMatchMenuWidget::SetUIEnabled(const bool bEnabled)
{
	CB_Start->SetIsEnabled(bEnabled);
	CB_Back->SetIsEnabled(bEnabled);
	CB_Invite->SetIsEnabled(bEnabled);
	CT_Code->SetIsEnabled(bEnabled);
}