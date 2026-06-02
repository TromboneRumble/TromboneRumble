#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "EasyMatchmakingManager.h"

void UMainUIRoot::Register()
{
	Super::Register();
	
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().AddDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingComplete().AddDynamic(this, &ThisClass::HandleMatchmakingComplete);
		MatchmakingManager->OnMatchmakingCanceled().AddDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}
}

void UMainUIRoot::HandleMatchmakingStarted()
{
	PushLoadingOverlay();
}

void UMainUIRoot::HandleMatchmakingComplete(const FName SessionName, const EEasyMatchmakingCompleteResult Result)
{
	if (Result == EEasyMatchmakingCompleteResult::Failure || Result == EEasyMatchmakingCompleteResult::NoResults)
	{
		PopLoadingOverlay();
	}
}


void UMainUIRoot::HandleMatchmakingCanceled()
{
	PopLoadingOverlay();
}
