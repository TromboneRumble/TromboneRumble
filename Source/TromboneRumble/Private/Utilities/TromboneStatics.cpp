// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Utilities/TromboneStatics.h"
#include "EasySessionStatics.h"
#include "EasySessionSubsystem.h"
#include "NativeGameplayTags.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/TromboneUISubsystem.h"
#include "UI/UserWidgets/Common/RootUI.h"
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

const FString UTromboneStatics::InGameMapSettingKey = TEXT("MAPNAME");

bool UTromboneStatics::CopyRoomCodeToClipboard(const UObject* WorldContextObject)
{
	const UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(WorldContextObject);
	const FString Code = Sessions ? Sessions->GetJoinCode() : FString();
	if (Code.IsEmpty())
	{
		return false;
	}

	FPlatformApplicationMisc::ClipboardCopy(*Code);
	return true;
}

FGameplayTag UTromboneStatics::GetCurrentInGameMap(const UObject* WorldContextObject, const FGameplayTag& Default)
{
	const UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(WorldContextObject);
	if (!Sessions)
	{
		return Default;
	}

	const FString MapName = Sessions->GetSessionSettings().CustomSettings.FindRef(InGameMapSettingKey);
	const FGameplayTag MapTag = MapName.IsEmpty() ? FGameplayTag() : FGameplayTag::RequestGameplayTag(FName(*MapName), false);
	return MapTag.IsValid() ? MapTag : Default;
}

FGameplayTag UTromboneStatics::GetMatchmakingFailureNotice(const EEasySessionResult Result)
{
	switch (Result)
	{
	case EEasySessionResult::Canceled:			return TromboneGamePlayTags::Trombone_UI_Notice_MatchmakingCanceled;
	case EEasySessionResult::NoSessionsFound:	return TromboneGamePlayTags::Trombone_UI_Notice_RoomNotFound;
	case EEasySessionResult::JoinSessionFull:	return TromboneGamePlayTags::Trombone_UI_Notice_RoomFull;
	case EEasySessionResult::JoinRefused:		return TromboneGamePlayTags::Trombone_UI_Notice_RoomUnavailable;
	default:									return TromboneGamePlayTags::Trombone_UI_Notice_MatchmakingFailed;
	}
}

int32 UTromboneStatics::GetMinPlayersToStart()
{
	return FMath::Max(1, CVarMinPlayersToStart.GetValueOnGameThread());
}

bool UTromboneStatics::HasEnoughPlayersToStart(const int32 PlayerCount)
{
	return PlayerCount >= GetMinPlayersToStart();
}

float UTromboneStatics::GetServerWorldTime(const UObject* WorldContextObject)
{
	// Gimmicks call it every frame, so a missing world returns quietly instead of logging
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return 0.f;

	const AGameStateBase* GameState = World->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
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
		case ELevelType::SpaceStation:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_InGame_SpaceStation);
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
		case ELevelType::SpaceStationLobby:
			MapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TromboneGamePlayTags::Trombone_Maps_Lobby_SpaceStation);
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

UCommonActivatableWidget* UTromboneStatics::ShowLoadingOverlay(const APlayerController* PlayerController)
{
	if (const URootUI* RootUI = GetRootUI(PlayerController))
	{
		if (UCommonActivatableWidget* LoadingOverlayWidget = RootUI->AddWidgetToStack(UTromboneConfig::Get()->LoadingWidgetClass.LoadSynchronous(), TromboneGamePlayTags::Trombone_UI_Layer_Overlay))
		{
			return LoadingOverlayWidget;
		}
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to show loading overlay"));
	return nullptr;
}

UFadeWidget* UTromboneStatics::ShowFadeOverlay(const APlayerController* PlayerController)
{
	if (const URootUI* RootUI = GetRootUI(PlayerController))
	{
		return Cast<UFadeWidget>(RootUI->AddWidgetToStack(UTromboneConfig::Get()->FadeWidgetClass.LoadSynchronous(), TromboneGamePlayTags::Trombone_UI_Layer_Overlay));
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to show fade overlay"));
	return nullptr;
}

bool UTromboneStatics::PopOverlay(const APlayerController* PlayerController)
{
	if (const URootUI* RootUI = GetRootUI(PlayerController))
	{
		return RootUI->PopStack(TromboneGamePlayTags::Trombone_UI_Layer_Overlay);
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to pop overlay"));
	return false;
}

URootUI* UTromboneStatics::GetRootUI(const APlayerController* PlayerController)
{
	if (PlayerController && PlayerController->GetLocalPlayer())
	{
		if (const UTromboneUISubsystem* UISubsystem = UTromboneUISubsystem::Get(PlayerController))
		{
			if (URootUI* RootUI = UISubsystem->GetRootUI())
			{
				return RootUI;
			}
		}
	}

	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to get RootUI"));
	return nullptr;
}
