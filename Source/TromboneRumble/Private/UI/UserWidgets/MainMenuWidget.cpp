// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenuWidget.h"

#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Subsystems/SessionSubsystem.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "TromboneGamePlayTags.h"
#include "Components/Slider.h"
#include "Components/SpinBox.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UserWidgets/ConfirmationDialogueWidget.h"
#include "Utilities/DebugHelper.h"

bool UMainMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	
	return true;
}

void UMainMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	const FString LobbyMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);
	checkf(!LobbyMapPath.IsEmpty(), TEXT("Lobby map path not found. Please set it in GameMapDeveloperSettings."));
	CachedLobbyMapPath = LobbyMapPath;

	BindSubsystemCallbacks();
	InitButtons();

	if (LobbyCodeText)
	{
		LobbyCodeText->SetText(FText::GetEmpty());
	}
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

void UMainMenuWidget::NativeDestruct()
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

void UMainMenuWidget::InitButtons()
{
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	if (MB_Option)
	{
		MB_Option->OnClicked().AddUObject(this, &ThisClass::HandleOptionButtonClicked);
	}
	if (MB_BackFromSettings)
	{
		MB_BackFromSettings->OnClicked().AddUObject(this, &ThisClass::HandleBackFromSettingsButtonClicked);
	}
	if (MB_Quit)
	{
		MB_Quit->OnClicked().AddUObject(this, &ThisClass::HandleQuitButtonClicked);
	}
}

void UMainMenuWidget::ChangePanel(UVerticalBox* TargetPanel)
{
	if (CAS_MainMenu)
	{
		CAS_MainMenu->SetActiveWidget(TargetPanel);
	}	
}

void UMainMenuWidget::BindSubsystemCallbacks()
{
	if (IsDesignTime()) return;
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

void UMainMenuWidget::RemoveSubsystemCallbacks()
{
	if (IsDesignTime()) return;
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

void UMainMenuWidget::OnCreateSession(bool bWasSuccessful)
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
		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
	}
}

void UMainMenuWidget::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (SessionsSubsystem == nullptr)
	{
		Debug::Print("SessionsSubsystem is nullptr from [OnFindSession]");
		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
		return;
	}
	const FString& LobbyCode = LobbyCodeText->GetText().ToString();

	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
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
	HostButton->SetIsEnabled(true);
	JoinButton->SetIsEnabled(true);
	
}


void UMainMenuWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	HostButton->SetIsEnabled(true);
	JoinButton->SetIsEnabled(true);

	// Join실패일 경우에는 이유 설명하고 리턴
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		const FString Reason = FString::Printf(
			TEXT("Join failed: %s (LAN=%d, OSS=%s)"),
			JoinSessionResultToText(Result),
			(int32)SessionsSubsystem->IsLanEnvironment(),
			IOnlineSubsystem::Get() ? *IOnlineSubsystem::Get()->GetSubsystemName().ToString() : TEXT("None"));
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

void UMainMenuWidget::OnDestroySession(bool bWasSuccessful)
{
}

void UMainMenuWidget::OnSessionError(const FString& Reason)
{
	Debug::Print(Reason);
}

void UMainMenuWidget::OnStartSession(bool bWasSuccessful)
{
}

void UMainMenuWidget::OnMaxPlayerSliderChanged(const float Value)
{
	if (MaxPlayerSpinBox)
	{
		MaxPlayerSpinBox->SetValue(FMath::RoundToInt(Value));
	}
}

void UMainMenuWidget::OnMaxPlayerSpinBoxChanged(const float Value)
{
	if (MaxPlayerSlider)
	{
		MaxPlayerSlider->SetValue(Value);
	}
}


void UMainMenuWidget::HostButtonClicked()
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
	HostButton->SetIsEnabled(false);
	JoinButton->SetIsEnabled(false);
	if (SessionsSubsystem)
	{
		SessionsSubsystem->CreateSession(NumPublicConnections, LobbyCode);
	}

}

void UMainMenuWidget::JoinButtonClicked()
{
	
	if (LobbyCodeText->GetText().IsEmpty())
	{
		Debug::Print(TEXT("Lobby Code is Empty from JoinButtonClicked"));
		return;
	}
	HostButton->SetIsEnabled(false);
	JoinButton->SetIsEnabled(false);
	if (SessionsSubsystem)
	{
		SessionsSubsystem->FindSessions(10000, LobbyCodeText->GetText().ToString().ToUpper());
	}
}

void UMainMenuWidget::HandleOptionButtonClicked()
{
	ChangePanel(VB_Settings);
}

void UMainMenuWidget::HandleBackFromSettingsButtonClicked()
{
	ChangePanel(VB_MainMenu);
}

void UMainMenuWidget::HandleQuitButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	UConfirmationDialogueWidget* Widget = CreateWidget<UConfirmationDialogueWidget>(PC, ConfirmationDialogueWidgetClass);

	// TODO : 메세지 관리
	const FText Message = FText::FromString(TEXT("Are you sure you want to quit the game?"));
	Widget->ShowDialogue(Message);
}

FString UMainMenuWidget::GenerateRandomLobbyCode(int32 Length)
{
	const FString Chars = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	FString RandomCode;
	for (int32 i = 0; i < Length; ++i)
	{
		RandomCode += Chars[FMath::RandRange(0, Chars.Len() - 1)];
	}
	return RandomCode;
}

const TCHAR* UMainMenuWidget::JoinSessionResultToText(const EOnJoinSessionCompleteResult::Type InResult) const
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
