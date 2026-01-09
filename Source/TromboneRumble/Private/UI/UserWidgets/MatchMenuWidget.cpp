// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MatchMenuWidget.h"

#include "CommonButtonBase.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/SpinBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SessionSubsystem.h"
#include "Utilities/DebugHelper.h"

class USessionSubsystem;

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
	UGameInstance* GameInstance = GetGameInstance();
	SessionsSubsystem = GameInstance->GetSubsystem<USessionSubsystem>();
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnSessionCreateComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		SessionsSubsystem->OnSessionSearchFinished.AddUObject(this, &ThisClass::OnFindSession);
		SessionsSubsystem->OnSessionJoinComplete.AddUObject(this, &ThisClass::OnJoinSession);
		SessionsSubsystem->OnSessionDestroyComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		SessionsSubsystem->OnSessionError.AddDynamic(this, &ThisClass::OnSessionError);
		SessionsSubsystem->OnSessionStart.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

void UMatchMenuWidget::RemoveSubsystemCallbacks()
{
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnSessionCreateComplete.RemoveDynamic(this, &ThisClass::OnCreateSession);
		SessionsSubsystem->OnSessionDestroyComplete.RemoveDynamic(this, &ThisClass::OnDestroySession);
		SessionsSubsystem->OnSessionError.RemoveDynamic(this, &ThisClass::OnSessionError);
		SessionsSubsystem->OnSessionStart.RemoveDynamic(this, &ThisClass::OnStartSession);
		SessionsSubsystem->OnSessionSearchFinished.RemoveAll(this);
		SessionsSubsystem->OnSessionJoinComplete.RemoveAll(this);
	}
}

void UMatchMenuWidget::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (UWorld* World = GetWorld())
		{
			const FString LobbyPkg = FPackageName::ObjectPathToPackageName(CachedLobbyMapPath);
			const FString URL = LobbyPkg + TEXT("?listen");
			Debug::Print(URL);
			UGameplayStatics::OpenLevel(this, FName(*URL), true);
		}
	}
	else
	{
		Debug::Print("Failed to Create Session! from [MainMenu.OnCreateSession]");
		CB_Host->SetIsEnabled(true);
		CB_Join->SetIsEnabled(true);
	}
}

void UMatchMenuWidget::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (SessionsSubsystem == nullptr)
	{
		Debug::Print("SessionsSubsystem is nullptr from [OnFindSession]");
		CB_Host->SetIsEnabled(true);
		CB_Join->SetIsEnabled(true);
		return;
	}
	const FString& LobbyCode = LobbyCodeText->GetText().ToString();

	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		CB_Host->SetIsEnabled(true);
		CB_Join->SetIsEnabled(true);
		Debug::Print(FString::Printf(TEXT("Failed to find session with LobbyCode %s on [OnFindSession]"), *LobbyCode));
		return;
	}

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		FString Code;
		//FString Kw, Map;
		Result.Session.SessionSettings.Get(USessionSubsystem::KEY_LOBBY_CODE, Code);
		//Result.Session.SessionSettings.Get(SEARCH_KEYWORDS, Kw); //로비 생성시 keyword 넣은 경우
		//Result.Session.SessionSettings.Get(SETTING_MAPNAME, Map); //로비 생성시 맵이름 넣은 경우
		const FString Owner = Result.Session.OwningUserName;
		const int32   Ping = Result.PingInMs;

		Debug::Print(FString::Printf(
			TEXT("Code=%s Owner=%s Ping=%d"),
			*Code, *Owner, Ping));

		Result.Session.SessionSettings.Get(SessionsSubsystem->KEY_LOBBY_CODE, SettingsValue);
		if (SettingsValue == LobbyCode)
		{
			Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
			Result.Session.SessionSettings.bUsesPresence = true;
			SessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	Debug::Print(FString::Printf(TEXT("No matched lobby code among results (wanted=%s)"), *LobbyCode));
	CB_Host->SetIsEnabled(true);
	CB_Join->SetIsEnabled(true);
	
}


void UMatchMenuWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	CB_Host->SetIsEnabled(true);
	CB_Join->SetIsEnabled(true);

	// Join실패일 경우에는 이유 설명하고 리턴
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		const FString Reason = FString::Printf(
			TEXT("Join failed: %s (LAN=%d, OSS=%s)"),
			JoinSessionResultToText(Result),
			static_cast<int32>(SessionsSubsystem->IsLanEnvironment()),
			Online::GetSubsystem(GetWorld()) ? *Online::GetSubsystem(GetWorld())->GetSubsystemName().ToString() : TEXT("None"));
		Debug::Print(Reason);
		return;
	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
	{
		Debug::Print(TEXT("MainMenu Error : No OnlineSubsystem found when trying to join from [OnJoinSession]"));
		return;
	}
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		Debug::Print(TEXT("MainMenu Error : No valid SessionInterface found when trying to join from [OnJoinSession]"));
		return;
	}

	// 실제 접속(메인메뉴 → 호스트의 Lobby 맵로 전환)
	FString Address;
	SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

	if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
	{
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		Debug::Print(TEXT("SessionSubsystem Error : No PlayerController to travel after join from [OnJoinSession]"));
		return;
	}

}

void UMatchMenuWidget::OnDestroySession(bool bWasSuccessful)
{
}

void UMatchMenuWidget::OnSessionError(const FString& Reason)
{
	Debug::Print(Reason);
}

void UMatchMenuWidget::OnStartSession(bool bWasSuccessful)
{
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
	NumPublicConnections = FMath::RoundToInt(MaxPlayerSlider->GetValue());
	
	FString LobbyCode;
	if (LobbyCodeText->GetText().IsEmpty())
	{
		LobbyCode = GenerateRandomLobbyCode(FMath::Max(2, MaxLobbyCodeLength));
	}
	else
	{
		LobbyCode = LobbyCodeText->GetText().ToString().ToUpper();
	}
	CB_Host->SetIsEnabled(false);
	CB_Join->SetIsEnabled(false);
	if (SessionsSubsystem)
	{
		SessionsSubsystem->CreateSession(NumPublicConnections, LobbyCode);
	}

}

void UMatchMenuWidget::JoinButtonClicked()
{
	
	if (LobbyCodeText->GetText().IsEmpty())
	{
		PRINT_WITH_CURRENT_CONTEXT("Lobby Code is Empty");
		return;
	}
	CB_Host->SetIsEnabled(false);
	CB_Join->SetIsEnabled(false);
	if (SessionsSubsystem)
	{
		SessionsSubsystem->FindSessions(10000, LobbyCodeText->GetText().ToString().ToUpper());
	}
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