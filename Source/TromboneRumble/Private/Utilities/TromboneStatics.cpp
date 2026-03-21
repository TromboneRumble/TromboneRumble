#include "Utilities/TromboneStatics.h"
#include "EnhancedInputSubsystems.h"
#include "NativeGameplayTags.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/BaseHUD.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "Utilities/DebugHelper.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "Utilities/Defines.h"

void UTromboneStatics::OpenLevel(const UObject* WorldContextObject, const ELevelState Level, const bool bAbsolute)
{
	FString MapPath;
	switch (Level)
	{
		case ELevelState::MainMenu:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MainMenu_Main);
			break;
		case ELevelState::Lobby:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Lobby_Main);
			break;
		case ELevelState::MatchMenu:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_MatchMenu_Main);
			break;
		case ELevelState::Tutorial:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_Tutorial_Main);
			break;
		case ELevelState::InGame:
			MapPath = UTromboneFunctionLibrary::GetMapPathByTag(TromboneGamePlayTags::Trombone_Maps_InGame_Main);
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::OpenLevel] Unknown level state"));
			return;
	}
	
	const FString URL = FPackageName::ObjectPathToPackageName(MapPath);
	UGameplayStatics::OpenLevel(WorldContextObject, FName(*URL), bAbsolute);
	
}

void UTromboneStatics::SetInputConfig(const UObject* WorldContextObject, bool bFocusUI, bool bShowCursor, bool bIgnoreInput, bool bRemoveMappingContext)
{
	// TODO : Common UI에 맞게 수정하고 테스트하기
	APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (!PC) return;
	
	if (bRemoveMappingContext)
	{
		if (const auto* LP = PC->GetLocalPlayer())
		{
			if (auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->ClearAllMappings(); 
			}
		}
	}

	PC->bShowMouseCursor = bShowCursor;
	if (bIgnoreInput)
	{
		PC->DisableInput(PC);
	}
	else
	{
		PC->EnableInput(PC);
	}

	if (bFocusUI)
	{
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Mode);
	}
	else
	{
		const FInputModeGameOnly Mode;
		PC->SetInputMode(Mode);
	}
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

UNoticePopupWidget* UTromboneStatics::ShowNoticePopup(const UObject* WorldContextObject)
{
	// TODO : 스택에 넣기
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	
	const UTromboneConfig* Config = UTromboneConfig::Get();
	UNoticePopupWidget* NoticePopup = CreateWidget<UNoticePopupWidget>(World, Config->NoticePopupWidgetClass);
	if (!NoticePopup)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to create NoticePopup"));
		return nullptr;
	}
	
	return NoticePopup;
}

UTwoButtonWithoutClosePopup* UTromboneStatics::ShowTwoButtonPopup(const UObject* WorldContextObject)
{
	// TODO : 스택에 넣기
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	const UTromboneConfig* Config = UTromboneConfig::Get();
	UTwoButtonWithoutClosePopup* Popup = CreateWidget<UTwoButtonWithoutClosePopup>(World, Config->TwoButtonWithoutClosePopupWidgetClass);
	if (!Popup)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to create TwoButtonWithoutClosePopup"));
		return nullptr;
	}
	
	return Popup;
}
