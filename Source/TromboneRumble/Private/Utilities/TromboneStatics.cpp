// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Utilities/TromboneStatics.h"
#include "EasyOnlineSession.h"
#include "EasySessions.h"
#include "NativeGameplayTags.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/ToastSubsystem.h"
#include "UI/HUD/BaseHUD.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "UI/UserWidgets/Common/FadeWidget.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

namespace
{
	constexpr int32 DefaultMinPlayersToStart = 2;
}

static TAutoConsoleVariable<int32> CVarMinPlayersToStart(
	TEXT("Trombone.MatchMenu.MinPlayersToStart"),
	DefaultMinPlayersToStart,
	TEXT("The minimum number of players required for a host to start a game from the Match menu. If set to 1, the host can start the game alone."));

FString UTromboneStatics::GenerateRandomRoomCode(const int32 CodeLength, const bool bClipboardCopy)
{
	const FString Chars = TEXT("ABCDEFGHJKMNPQRSTUVWXYZ23456789");
	FString RandomCode;
	for (int32 i = 0; i < CodeLength; ++i)
	{
		RandomCode += Chars[FMath::RandRange(0, Chars.Len() - 1)];
	}
	
	if (bClipboardCopy)
	{
		FPlatformApplicationMisc::ClipboardCopy(*RandomCode);
	}
	
	return RandomCode;
}

bool UTromboneStatics::CopyRoomCodeToClipboard(const UObject* WorldContextObject)
{
	if (GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(WorldContextObject))
		{
			FString Code;
			if (OnlineSession->GetSessionSetting(NAME_GameSession, SETTING_LOBBYCODE, Code))
			{
				FPlatformApplicationMisc::ClipboardCopy(*Code);
				return true;
			}
		}
	}
	
	return false;
}

int32 UTromboneStatics::GetMinPlayersToStart()
{
	return FMath::Max(1, CVarMinPlayersToStart.GetValueOnGameThread());
}

bool UTromboneStatics::HasEnoughPlayersToStart(const int32 PlayerCount)
{
	return PlayerCount >= GetMinPlayersToStart();
}

void UTromboneStatics::OpenLevel(const UObject* WorldContextObject, const ELevelType Level, const bool bAbsolute)
{
	FString MapPath;
	switch (Level)
	{
		// InGame
		case ELevelType::OrchestraStage:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_InGame_OrchestraStage);
			break;
		case ELevelType::SnowField:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_InGame_SnowField);
			break;
		case ELevelType::JazzBar:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_InGame_JazzBar);
			break;

		// Lobby
		case ELevelType::OrchestraStageLobby:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_Lobby_OrchestraStage);
			break;
		case ELevelType::SnowFieldLobby:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_Lobby_SnowField);
			break;
		case ELevelType::JazzBarLobby:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_Lobby_JazzBar);
			break;
		
		// OutGame
		case ELevelType::MainMenu:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_MainMenu);
			break;
		case ELevelType::MatchMenu:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_MatchMenu);
			break;
		case ELevelType::Tutorial:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_Tutorial);
			break;
		case ELevelType::Customize:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_Customize);
			break;

		// Result
		// 모든 Result 태그는 동일한 레벨을 가리키므로, 그중 어느 것을 사용하더라도 올바른 경로를 반환
		// 스테이지를 이어가야 할 때는 UResultSceneSubsystem::OpenResultLevel을 사용
		case ELevelType::Result:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_Result_OrchestraStage);
			break;

		default:
			UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::OpenLevel] Unknown level state. Traveling Main Menu"));
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_OutGame_MainMenu);
			return;
	}
	
	const FString URL = FPackageName::ObjectPathToPackageName(MapPath);
	UGameplayStatics::OpenLevel(WorldContextObject, FName(*URL), bAbsolute);
}

FToastRequest UTromboneStatics::MakeToastRequest(const FText& Message, const EToastPosition Position, const float DisplayDuration)
{
	return FToastRequest(Message, Position, DisplayDuration);
}

bool UTromboneStatics::ShowToast(const UObject* WorldContextObject, const FToastRequest Request)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UToastSubsystem* ToastSys = World->GetGameInstance()->GetSubsystem<UToastSubsystem>())
		{
			ToastSys->ShowToast(Request);
			
			return true;
		}
	}
	
	return false;
}

UCommonActivatableWidget* UTromboneStatics::ShowLoadingOverlay(const APlayerController* PlayerController)
{
	if (const UBaseUIRoot* RootLayout = GetRootLayout(PlayerController))
	{
		if (UCommonActivatableWidget* LoadingOverlayWidget = RootLayout->AddWidgetToStack(UTromboneConfig::Get()->LoadingWidgetClass, EUIStackType::Overlay))
		{
			return LoadingOverlayWidget;
		}
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to show loading overlay"));
	return nullptr;
}

UFadeWidget* UTromboneStatics::ShowFadeOverlay(const APlayerController* PlayerController)
{
	if (const UBaseUIRoot* RootLayout = GetRootLayout(PlayerController))
	{
		return Cast<UFadeWidget>(RootLayout->AddWidgetToStack(UTromboneConfig::Get()->FadeWidgetClass, EUIStackType::Overlay));
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to show fade overlay"));
	return nullptr;
}

bool UTromboneStatics::PopOverlay(const APlayerController* PlayerController)
{
	if (const UBaseUIRoot* RootLayout = GetRootLayout(PlayerController))
	{
		return RootLayout->PopStack(EUIStackType::Overlay);
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to pop overlay"));
	return false;
}

UBaseUIRoot* UTromboneStatics::GetRootLayout(const APlayerController* PlayerController)
{
	if (PlayerController && PlayerController->GetLocalPlayer())
	{
		if (const ABaseHUD* Hud = Cast<ABaseHUD>(PlayerController->GetHUD()))
		{
			if (UBaseUIRoot* RootLayout = Cast<UBaseUIRoot>(Hud->GetRootUI()))
			{
				return RootLayout;
			}
		}
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to get RootLayout"));
	return nullptr;
}
