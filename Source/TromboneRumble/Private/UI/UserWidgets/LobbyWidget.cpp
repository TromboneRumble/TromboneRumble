// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/LobbyWidget.h"
#include "EasySessionSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "TromboneGamePlayTags.h"
#include "Framework/LobbyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

bool ULobbyWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	const FString InGameMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_InGame_Main);
	checkf(!InGameMapPath.IsEmpty(), TEXT("InGameMapPath map path not found. Please set it in GameMapDeveloperSettings."));
	CachedInGameMapPath = InGameMapPath;

	const FString MainMenuMapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
	checkf(!MainMenuMapPath.IsEmpty(), TEXT("MainMenu map path not found. Please set it in GameMapDeveloperSettings."));
	CachedMainMenuMapPath = MainMenuMapPath;
	
	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->OnLobbyStateChanged.AddDynamic(this, &ThisClass::OnLobbyStateUpdated);
		OnPlayerListUpdated(LobbyGameState->GetPlayerList());
	}

	return true;
}

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	BindSubsystemCallbacks();

	if (CountdownText)
	{
		CountdownText->SetVisibility(ESlateVisibility::Hidden);
	}

	const FString LobbyCode = SessionsSubsystem->GetCurrentSessionProperty(GKey_Lobby_Code.ToString());
	if (!LobbyCode.IsEmpty())
	{
		LobbyText->SetText(FText::FromString(LobbyCode));
	}
	else
	{
		LobbyText->SetText(FText::FromString(TEXT("No Lobby Code")));
	}

	if (SessionsSubsystem->IsAdmin())
	{
		IsHostText->SetText(FText::FromString(TEXT("Host")));
	}
	else
	{
		IsHostText->SetText(FText::FromString(TEXT("Client")));
	}
}

void ULobbyWidget::NativeDestruct()
{
	RemoveSubsystemCallbacks();

	Super::NativeDestruct();
}

void ULobbyWidget::BindSubsystemCallbacks()
{
	const UGameInstance* GameInstance = GetGameInstance();
	SessionsSubsystem = GameInstance->GetSubsystem<UEasySessionSubsystem>();

	RemoveSubsystemCallbacks();
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnDestroySessionSuccess.AddUObject(this, &ThisClass::OnDestroySessionSuccess);
		SessionsSubsystem->OnDestroySessionFailure.AddUObject(this, &ThisClass::OnDestroySessionFailure);
		// SessionsSubsystem->OnPlayerListUpdated.AddDynamic(this, &ThisClass::OnPlayerListUpdated);
	}
}

void ULobbyWidget::RemoveSubsystemCallbacks()
{
	if (SessionsSubsystem)
	{
		SessionsSubsystem->OnDestroySessionSuccess.RemoveAll(this);
		SessionsSubsystem->OnDestroySessionFailure.RemoveAll(this);
		// SessionsSubsystem->OnPlayerListUpdated.RemoveAll(this);
	}
}

void ULobbyWidget::OnDestroySessionSuccess()
{
	PRINT_WITH_CURRENT_CONTEXT("Session destroyed successfully, returning to Main Menu");
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

void ULobbyWidget::OnDestroySessionFailure()
{
	PRINT_WITH_CURRENT_CONTEXT("Failed to destroy session");
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

void ULobbyWidget::OnLobbyStateUpdated(const ELobbyState NewState)
{
	if (!CountdownText) return;
	
	if (NewState == ELobbyState::CountdownToScramble || NewState == ELobbyState::CountdownToTravel)
	{
		CountdownSeconds = 5; // TODO : delete magic number
		CountdownText->SetText(FText::AsNumber(CountdownSeconds));
		CountdownText->SetVisibility(ESlateVisibility::Visible);
		GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ULobbyWidget::UpdateCountdown, 1.0f, true);
	}
	else
	{
		CountdownText->SetVisibility(ESlateVisibility::Hidden);
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
}

void ULobbyWidget::UpdateCountdown()
{
	if (!CountdownText) return;

	CountdownSeconds--;
	CountdownText->SetText(FText::AsNumber(CountdownSeconds));

	if (CountdownSeconds <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
		CountdownText->SetVisibility(ESlateVisibility::Hidden);
	}
}