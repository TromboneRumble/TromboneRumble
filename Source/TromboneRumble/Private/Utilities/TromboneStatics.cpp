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
#include "UI/UserWidgets/Common/LoadingOverlayWidget.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

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

void UTromboneStatics::OpenLevel(const UObject* WorldContextObject, const ELevelType Level, const bool bAbsolute)
{
	FString MapPath;
	switch (Level)
	{
		case ELevelType::MainMenu:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
			break;
		case ELevelType::Lobby:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);
			break;
		case ELevelType::MatchMenu:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MatchMenu_Main);
			break;
		case ELevelType::Tutorial:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Tutorial_Main);
			break;
		case ELevelType::InGame:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_InGame_Main);
			break;
		case ELevelType::Customize:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Customize_Main);
			break;
		case ELevelType::ResultScene:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_ResultScene_Main);
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::OpenLevel] Unknown level state"));
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

ULoadingOverlayWidget* UTromboneStatics::ShowLoadingOverlay(const APlayerController* PlayerController)
{
	if (const UBaseUIRoot* RootLayout = GetRootLayout(PlayerController))
	{
		if (UCommonActivatableWidget* AddedWidget = RootLayout->AddWidgetToStack(UTromboneConfig::Get()->LoadingWidgetClass, EUIStackType::Overlay))
		{
			if (ULoadingOverlayWidget* LoadingWidget = Cast<ULoadingOverlayWidget>(AddedWidget))
			{
				return LoadingWidget;
			}
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
