// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenuWidget.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "EasySessionSettings.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Framework/TromboneGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/DebugHelper.h"

void UMatchMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (CT_Code)
	{
		CT_Code->SetText(FText::GetEmpty());
	}
}

void UMatchMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	const FString MainMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
	checkf(!MainMenuMapPath.IsEmpty(), TEXT("Main menu map path not found. Please set it in GameMapDeveloperSettings."));
	CachedMainMenuMapPath = MainMenuMapPath;
	
	const FString LobbyMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);
	checkf(!LobbyMapPath.IsEmpty(), TEXT("Lobby map path not found. Please set it in GameMapDeveloperSettings."));
	CachedLobbyMapPath = LobbyMapPath;
}

void UMatchMenuWidget::NativeDestruct()
{
	RemoveFromParent();
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	RemoveSubsystemCallbacks();

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
				FString OutCode;
				Setting->Data.GetValue(OutCode);
				CT_Code->SetText(FText::FromString(OutCode));
			}
		}
	}
}

UWidget* UMatchMenuWidget::NativeGetDesiredFocusTarget() const
{
	if (CB_Start)
	{
		return CB_Start;
	}
	return Super::NativeGetDesiredFocusTarget();
}

void UMatchMenuWidget::Init()
{
	if (CB_Start)
	{
		CB_Start->OnClicked().RemoveAll(this);
		CB_Start->OnClicked().AddUObject(this, &ThisClass::HandleStartButtonClicked);
	}
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddLambda([this]
		{
			const FString MainMenuPkg = FPackageName::ObjectPathToPackageName(CachedMainMenuMapPath);
			const FString URL = MainMenuPkg;
			UGameplayStatics::OpenLevel(this, FName(*URL), true);
		});
	}
}

void UMatchMenuWidget::HandleStartButtonClicked()
{
	if (UTromboneGameInstance* TromboneGI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		if (const UGameStateSubsystem* GameStateSubsystem = TromboneGI->GetSubsystem<UGameStateSubsystem>())
		{
			const FString MapPath = GameStateSubsystem->GetMapNameForTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);

			UWorld* World = GetWorld();
			if (!World || World->GetAuthGameMode() == nullptr || MapPath.IsEmpty()) return;
	
			if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
			{
				const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
				if (SessionInterface.IsValid())
				{
					if (const FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession))
					{
						TromboneGI->SetSessionPlayerNumber(Session->RegisteredPlayers.Num());
						PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Registered Player Count: %d"), TromboneGI->GetSessionPlayerNumber()));
					}
				}
			}
			
			if (!World->ServerTravel(MapPath))
			{
				PRINT_WITH_CURRENT_CONTEXT(TEXT("ServerTravel failed"));
			}
		}
	}
}

void UMatchMenuWidget::SetUIEnabled(const bool bEnabled)
{
	CB_Start->SetIsEnabled(bEnabled);
	CB_Back->SetIsEnabled(bEnabled);
	CT_Code->SetIsEnabled(bEnabled);
}