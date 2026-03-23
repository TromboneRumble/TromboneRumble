#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "EasyMatchmakingManager.h"
#include "EasyPartyManager.h"

void UMainUIRoot::Register()
{
	Super::Register();
	
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().AddDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingComplete().AddDynamic(this, &ThisClass::HandleMatchmakingCompleted);
		MatchmakingManager->OnMatchmakingCanceled().AddDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}
}

void UMainUIRoot::HandleMatchmakingStarted()
{
	PushLoadingOverlay();
}

void UMainUIRoot::HandleMatchmakingCompleted(FName SessionName, EEasyMatchmakingCompleteResult Result)
{
	PopLoadingOverlay();
}

void UMainUIRoot::HandleMatchmakingCanceled()
{
	PopLoadingOverlay();
}
