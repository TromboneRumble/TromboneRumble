// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenuWidget.h"
#include "CommonButtonBase.h"
#include "EasySessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/SpinBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"

void UMatchMenuWidget::Init(const TFunction<void()> OnMenuClosedCallback)
{
	OnMenuClosed = OnMenuClosedCallback;
}

void UMatchMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (LobbyCodeText)
	{
		LobbyCodeText->SetText(FText::GetEmpty());
	}
}

void UMatchMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	const FString LobbyMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);
	checkf(!LobbyMapPath.IsEmpty(), TEXT("Lobby map path not found. Please set it in GameMapDeveloperSettings."));
	CachedLobbyMapPath = LobbyMapPath;
	
	BindSubsystemCallbacks();
	InitButtons();
	
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	
	if (MaxPlayerSlider && MaxPlayerSpinBox)
	{
		MaxPlayerSlider->OnValueChanged.AddDynamic(this, &ThisClass::OnMaxPlayerSliderChanged);
		MaxPlayerSpinBox->OnValueChanged.AddDynamic(this, &ThisClass::OnMaxPlayerSpinBoxChanged);
		MaxPlayerSpinBox->SetValue(MaxPlayerSlider->GetValue());
	}
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

void UMatchMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	if (OnMenuClosed) OnMenuClosed();
}

UWidget* UMatchMenuWidget::NativeGetDesiredFocusTarget() const
{
	return CB_Host;
}

void UMatchMenuWidget::InitButtons()
{
	if (CB_Host)
	{
		CB_Host->OnClicked().RemoveAll(this);
		CB_Host->OnClicked().AddUObject(this, &ThisClass::HostButtonClicked);
	}
	if (CB_Join)
	{
		CB_Join->OnClicked().RemoveAll(this);
		CB_Join->OnClicked().AddUObject(this, &ThisClass::JoinButtonClicked);
	}
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddLambda([this]
		{
			if (OnMenuClosed) OnMenuClosed();
		});
	}
}

void UMatchMenuWidget::BindSubsystemCallbacks()
{
	if (!SessionsSubsystem)
	{
		const UGameInstance* GameInstance = GetGameInstance();
		SessionsSubsystem = GameInstance->GetSubsystem<UEasySessionSubsystem>();
	}
	
	RemoveSubsystemCallbacks();
	
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
}

void UMatchMenuWidget::RemoveSubsystemCallbacks()
{
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnStartSessionSuccess.RemoveAll(this);
		SessionsSubsystem->OnStartSessionFailure.RemoveAll(this);
		
		SessionsSubsystem->OnFindSessionsSuccess.RemoveAll(this);
		SessionsSubsystem->OnFindSessionsFailure.RemoveAll(this);
		
		SessionsSubsystem->OnJoinSessionSuccess.RemoveAll(this);
		SessionsSubsystem->OnJoinSessionFailure.RemoveAll(this);
		
		SessionsSubsystem->OnDestroySessionSuccess.RemoveAll(this);
		SessionsSubsystem->OnDestroySessionFailure.RemoveAll(this);
	}
}

void UMatchMenuWidget::OnStartSessionSuccess()
{
	const FString LobbyPkg = FPackageName::ObjectPathToPackageName(CachedLobbyMapPath);
	const FString URL = LobbyPkg + TEXT("?listen");
	UGameplayStatics::OpenLevel(this, FName(*URL), true);
}

void UMatchMenuWidget::OnStartSessionFailure()
{
	MatchButtonsSetEnabled(true);
	PRINT_WITH_CURRENT_CONTEXT("Failed to Start Session");
}

void UMatchMenuWidget::OnFindSessionsSuccess(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	const FString& LobbyCode = LobbyCodeText->GetText().ToString();

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		FString Code;
		Result.Session.SessionSettings.Get(KEY_LOBBY_CODE, Code);
		const FString Owner = Result.Session.OwningUserName;
		const int32   Ping = Result.PingInMs;

		Debug::Print(FString::Printf(TEXT("Code=%s Owner=%s Ping=%d"), *Code, *Owner, Ping));

		Result.Session.SessionSettings.Get(KEY_LOBBY_CODE, SettingsValue);
		if (SettingsValue == LobbyCode)
		{
			Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
			Result.Session.SessionSettings.bUsesPresence = true;
			SessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	
	Debug::Print(FString::Printf(TEXT("No matched lobby code among results (wanted=%s)"), *LobbyCode));
	MatchButtonsSetEnabled(true);
}

void UMatchMenuWidget::OnFindSessionsFailure(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	MatchButtonsSetEnabled(true);
	PRINT_WITH_CURRENT_CONTEXT("Failed to find sessions");
}

void UMatchMenuWidget::OnJoinSessionSuccess()
{
	MatchButtonsSetEnabled(true);
}

void UMatchMenuWidget::OnJoinSessionFailure()
{
	MatchButtonsSetEnabled(true);
	PRINT_WITH_CURRENT_CONTEXT("Join Failed");
}

void UMatchMenuWidget::OnDestroySessionSuccess()
{
	PRINT_WITH_CURRENT_CONTEXT("Session destroyed successfully");
}

void UMatchMenuWidget::OnDestroySessionFailure()
{
	PRINT_WITH_CURRENT_CONTEXT("Failed to destroy session");
}

void UMatchMenuWidget::OnMaxPlayerSliderChanged(const float Value)
{
	if (MaxPlayerSpinBox)
	{
		MaxPlayerSpinBox->SetValue(FMath::RoundToInt(Value));
	}
}

void UMatchMenuWidget::OnMaxPlayerSpinBoxChanged(const float Value)
{
	if (MaxPlayerSlider)
	{
		MaxPlayerSlider->SetValue(Value);
	}
}

void UMatchMenuWidget::HostButtonClicked()
{
	FString LobbyCode;
	if (LobbyCodeText->GetText().IsEmpty())
	{
		LobbyCode = GenerateRandomLobbyCode(FMath::Max(2, MaxLobbyCodeLength));
	}
	else
	{
		LobbyCode = LobbyCodeText->GetText().ToString().ToUpper();
	}
	
	if (SessionsSubsystem)
	{
		MatchButtonsSetEnabled(false);

		FEasySessionSettings Settings;
		Settings.NumPublicConnections = FMath::RoundToInt(MaxPlayerSlider->GetValue());
		Settings.CustomProperties.Add(KEY_LOBBY_CODE.ToString(), LobbyCode);
		SessionsSubsystem->CreateSession(Settings);
	}
}

void UMatchMenuWidget::JoinButtonClicked()
{
	if (LobbyCodeText->GetText().IsEmpty())
	{
		PRINT_WITH_CURRENT_CONTEXT("Lobby Code is Empty");
		return;
	}
	
	if (SessionsSubsystem)
	{
		MatchButtonsSetEnabled(false);

		FEasySearchSettings SearchSettings;
		SearchSettings.QuerySettings.Add(KEY_LOBBY_CODE.ToString(), LobbyCodeText->GetText().ToString().ToUpper());
		SessionsSubsystem->FindSessions(SearchSettings);
	}
}

void UMatchMenuWidget::MatchButtonsSetEnabled(const bool bEnabled)
{
	CB_Host->SetIsEnabled(bEnabled);
	CB_Join->SetIsEnabled(bEnabled);
}

FString UMatchMenuWidget::GenerateRandomLobbyCode(const int32 Length) const
{
	const FString Chars = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	FString RandomCode;
	for (int32 i = 0; i < Length; ++i)
	{
		RandomCode += Chars[FMath::RandRange(0, Chars.Len() - 1)];
	}
	
	FPlatformApplicationMisc::ClipboardCopy(*RandomCode);
	PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Lobby Code copied to Clipboard: %s"), *RandomCode));
	
	return RandomCode;
}

const TCHAR* UMatchMenuWidget::JoinSessionResultToText(const EOnJoinSessionCompleteResult::Type InResult) const
{
	switch (InResult)
	{
	case EOnJoinSessionCompleteResult::Success:               return TEXT("Success");
	case EOnJoinSessionCompleteResult::SessionIsFull:         return TEXT("SessionIsFull");
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:   return TEXT("SessionDoesNotExist");
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:return TEXT("CouldNotRetrieveAddress");
	case EOnJoinSessionCompleteResult::AlreadyInSession:      return TEXT("AlreadyInSession");
	default:                                                  return TEXT("Unknown");
	}
}