#include "UI/UserWidgets/MatchMenu/MatchMenuWidget.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "EasyMatchmakingManager.h"
#include "EasyMatchmakingPolicy.h"
#include "EasyOnlineSession.h"
#include "EasyReservationManager.h"
#include "EasySessionSettings.h"
#include "EasySessionUtils.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/Button.h"
#include "Framework/TromboneGameInstance.h"
#include "Framework/GameState/MatchMenuGameState.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
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
	
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr SessionInterface = OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Session interface is not valid"));
		return;
	}
	
	FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!NamedSession)
	{
		UE_LOG(LogTemp, Error, TEXT("No session data found"));
		return;
	}
	
	FString OutCode;
	if (!NamedSession->SessionSettings.Get(GKey_Lobby_Code, OutCode))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get lobby code from session settings"));
		return;
	}
	
	if (!CT_Code)
	{
		UE_LOG(LogTemp, Error, TEXT("CT_Code is not bound in the widget"));
		return;
	}

	CT_Code->SetText(FText::FromString(OutCode));
}

void UMatchMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this);
	MatchmakingManager->OnMatchmakingUpdated().AddDynamic(this, &ThisClass::HandleMatchmakingUpdated);
}

void UMatchMenuWidget::Init()
{
	Super::Init();
	
	const APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	const bool bIsHost = PC->HasAuthority();
	
	if (CB_Start)
	{
		CB_Start->OnClicked().RemoveAll(this);
		CB_Start->OnClicked().AddUObject(this, &ThisClass::HandleStartButtonClicked);
		CB_Start->SetIsEnabled(bIsHost);
	}
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddUObject(this, &ThisClass::HandleBackButtonClicked);
	}
	if (CR_MatchType)
	{
		CR_MatchType->OnRotatedWithDirection().RemoveAll(this);
		CR_MatchType->OnRotatedWithDirection().AddDynamic(this, &ThisClass::HandleOnRotatedMatchType);
		CR_MatchType->SetIsEnabled(bIsHost);
	}
}

void UMatchMenuWidget::BindGameStateEvents()
{
	RemoveGameStateEvents();
	
	if (AMatchMenuGameState* MatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGS->OnMatchTypeChanged.AddDynamic(this, &ThisClass::OnMatchTypeChanged);
		OnMatchTypeChanged(MatchMenuGS->GetCurrentMatchType());
	}
}

void UMatchMenuWidget::RemoveGameStateEvents()
{
	if (AMatchMenuGameState* MatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGS->OnMatchTypeChanged.RemoveAll(this);
	}
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
	SetUIEnabled(false);
	
	bIsStarted = true;
	UEasyReservationManager* ReservationManager = UEasyReservationManager::Get(this);
	if (ReservationManager->IsReservationHost())
	{ 
		TArray<FEasyReservation> Reservations = ReservationManager->CopyRegisteredReservations();
		ReservationManager->SetHostReservations(Reservations);
	}
	
	FEasySessionSettings UpdatedSettings;
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
			
	OnlineSession->GetSessionSettings(NAME_GameSession, UpdatedSettings);
	UpdatedSettings.bAllowJoinInProgress = false;
		
	OnlineSession->UpdateSession(NAME_GameSession, UpdatedSettings, true);
	OnlineSession->StartOnlineSession(NAME_GameSession);
	
	FString URL = TEXT("/Game/Levels/LobbyMap");
	UEasyStatics::ServerTravelToLevel(this, URL);
}

void UMatchMenuWidget::HandleBackButtonClicked()
{
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
	OnlineSession->DestroySession(NAME_GameSession);
	
	const FString MainMenuPkg = FPackageName::ObjectPathToPackageName(CachedMainMenuMapPath);
	const FString URL = MainMenuPkg;
	UGameplayStatics::OpenLevel(this, FName(*URL), true);
}

void UMatchMenuWidget::HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir)
{
	if (AMatchMenuGameState* MatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		EMatchType Type = static_cast<EMatchType>(Value);
		MatchMenuGS->SetMatchType(Type);
		
		ShowLoadingOverlay();
		
		bool bNewHidden = false;
		switch (Type)
		{
			case EMatchType::Public:
				bNewHidden = false;
				break;
			
			case EMatchType::Custom:
				bNewHidden = true;
				break;
			
			default: 
				break;
		}
		
		FEasySessionSettings UpdatedSettings;
		UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
			
		OnlineSession->OnUpdateMatchComplete().AddDynamic(this, &ThisClass::HandleOnUpdateCompleteInMatchmaking);
		OnlineSession->GetSessionSettings(NAME_GameSession, UpdatedSettings);
		UpdatedSettings.bHidden = bNewHidden;
		
		TArray<FEasySessionSetting> ExtraSessionSettings = TArray<FEasySessionSetting>();
		
		FString LobbyCode;
		if (UpdatedSettings.GetSessionSetting(GKey_Lobby_Code, LobbyCode))
		{
			ExtraSessionSettings.Add(FEasySessionSetting(GKey_Lobby_Code, LobbyCode, EOnlineDataAdvertisementType::ViaOnlineService));
		}
				
		OnlineSession->UpdateSession(NAME_GameSession, UpdatedSettings, true, ExtraSessionSettings);
	}
}

void UMatchMenuWidget::HandleMatchmakingUpdated(const EEasyMatchmakingState MatchmakingState, const int32 MatchmakingTime)
{
	if (UEasyStatics::IsMatchmaking(GetWorld()))
	{
		ShowLoadingOverlay();
	}
	else
	{
		HideLoadingOverlay();
	}
}

void UMatchMenuWidget::HandleOnUpdateCompleteInMatchmaking(bool bWasSuccessful)
{
	HideLoadingOverlay();
	
	if (bWasSuccessful)
	{
		UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
		OnlineSession->OnUpdateMatchComplete().RemoveDynamic(this, &ThisClass::HandleOnUpdateCompleteInMatchmaking);
	}
}

void UMatchMenuWidget::SetUIEnabled(const bool bEnabled)
{
	Super::SetUIEnabled(bEnabled);
	
	CB_Start->SetIsEnabled(bEnabled);
	CB_Back->SetIsEnabled(bEnabled);
}