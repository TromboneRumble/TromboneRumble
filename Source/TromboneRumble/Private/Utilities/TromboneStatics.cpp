#include "Utilities/TromboneStatics.h"
#include "NativeGameplayTags.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/BaseHUD.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "Utilities/DebugHelper.h"
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