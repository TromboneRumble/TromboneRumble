// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/LobbyWidget.h"
#include "LobbyGameState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/SessionSubsystem.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "TromboneGamePlayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"



bool ULobbyWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	const FString InGameMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_InGameMap);
	checkf(!InGameMapPath.IsEmpty(), TEXT("InGameMapPath map path not found. Please set it in GameMapDeveloperSettings."));
	CachedInGameMapPath = InGameMapPath;

	const FString MainMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMap);
	checkf(!MainMenuMapPath.IsEmpty(), TEXT("MainMenu map path not found. Please set it in GameMapDeveloperSettings."));
	CachedMainMenuMapPath = MainMenuMapPath;

	BindSubsystemCallbacks();

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &ThisClass::StartGameButtonClicked);
	}
	if (BackToMainMenuButton)
	{
		BackToMainMenuButton->OnClicked.AddDynamic(this, &ThisClass::BackToMainMenuButtonClicked);
	}


	

	return true;
}


void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!GetWorld()) return;
	
	checkf(SessionsSubsystem, TEXT("SessionsSubsystem is null from [NativeConstruct]"));
	
	if (!SessionsSubsystem->IsLocalHost())
	{
		StartGameButton->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		StartGameButton->SetVisibility(ESlateVisibility::Visible);
	}

	FString LobbyCode;
	if (SessionsSubsystem->TryGetCurrentLobbyCode(LobbyCode))
	{
		LobbyText->SetText(FText::FromString(LobbyCode));
	}
	else
	{
		Debug::Print(TEXT("Failed to get LobbyCode from [NativeConstruct]"));
		LobbyText->SetText(FText::FromString(TEXT("ERROR")));
	}

	if (SessionsSubsystem->IsLocalHost())
	{
		IsHostText->SetText(FText::FromString(TEXT("Host")));
	}
	else
	{
		IsHostText->SetText(FText::FromString(TEXT("Client")));
	}

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
}

void ULobbyWidget::NativeDestruct()
{
	RemoveSubsystemCallbacks();
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	Super::NativeDestruct();
}

void ULobbyWidget::BindSubsystemCallbacks()
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
		SessionsSubsystem->OnPlayerListUpdated.AddDynamic(this, &ThisClass::OnPlayerListUpdated);
	}
}

void ULobbyWidget::RemoveSubsystemCallbacks()
{
	if (IsDesignTime()) return;
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnSessionCreateComplete.RemoveDynamic(this, &ThisClass::OnCreateSession);
		SessionsSubsystem->OnSessionDestroyComplete.RemoveDynamic(this, &ThisClass::OnDestroySession);
		SessionsSubsystem->OnSessionError.RemoveDynamic(this, &ThisClass::OnSessionError);
		SessionsSubsystem->OnSessionStart.RemoveDynamic(this, &ThisClass::OnStartSession);
		SessionsSubsystem->OnSessionSearchFinished.RemoveAll(this); // AddUObject는 RemoveAll/Handle 필요
		SessionsSubsystem->OnSessionJoinComplete.RemoveAll(this);
		SessionsSubsystem->OnPlayerListUpdated.RemoveDynamic(this, &ThisClass::OnPlayerListUpdated);
	}
}


void ULobbyWidget::OnCreateSession(bool bWasSuccessful)
{
}

void ULobbyWidget::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
}

void ULobbyWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
}

void ULobbyWidget::OnDestroySession(bool bWasSuccessful)
{
	FString PackagePath = CachedMainMenuMapPath;
	if (PackagePath.Contains(TEXT(".")))
	{
		PackagePath = FSoftObjectPath(CachedMainMenuMapPath).GetLongPackageName();
		if (PackagePath.IsEmpty())
		{
			Debug::Print(TEXT("Invalid MapPath from [LobbyWidget : OnDestroySession]"));
			return;
		}
	}
	UGameplayStatics::OpenLevel(GetWorld(), FName(*PackagePath), true);
}

void ULobbyWidget::OnSessionError(const FString& Reason)
{
	Debug::Print(FString::Printf(TEXT("Session Error: %s from [OnSessionError]"), *Reason));
}

void ULobbyWidget::OnStartSession(bool bWasSuccessful)
{
}

void ULobbyWidget::OnPlayerListUpdated(const TArray<FString>& PlayerNames)
{
	if (!PlayerListText) return;

	FString FormattedPlayerList = TEXT("Players:\n");

	for (int32 i = 0; i < PlayerNames.Num(); ++i)
	{
		FormattedPlayerList.Append(FString::Printf(TEXT("%d. %s\n"), i + 1, *PlayerNames[i]));
	}
	
	PlayerListText->SetText(FText::FromString(FormattedPlayerList));
}

void ULobbyWidget::StartGameButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Debug::Print(TEXT("World is null from [StartGameButtonClicked]"));
		return;
	}

	// 전달받은 문자열이 "오브젝트 경로(/A/B.Map.Map)"면 패키지 경로("/A/B.Map")로 정규화
	FString PackagePath = CachedInGameMapPath;
	if (PackagePath.Contains(TEXT(".")))
	{
		PackagePath = FSoftObjectPath(CachedInGameMapPath).GetLongPackageName();
		if (PackagePath.IsEmpty())
		{
			Debug::Print(TEXT("Invalid MapPath from [StartGameButtonClicked]"));
			return;
		}
	}

	//호스트(리스닝 서버)라면 연결 중인 모든 클라와 함께 이동
	if (World->GetAuthGameMode() == nullptr)
	{
		Debug::Print(TEXT("StartGame can be called only on host from [StartGameButtonClicked]"));
		return;
	}

	if (!World->ServerTravel(PackagePath))
	{
		Debug::Print(TEXT("ServerTravel failed from [StartGameButtonClicked]"));
	}
}

void ULobbyWidget::BackToMainMenuButtonClicked()
{
	SessionsSubsystem->DestroySession();
}
