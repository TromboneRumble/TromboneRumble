#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "EasyMatchmakingManager.h"

void UMainUIRoot::Register()
{
	Super::Register();
	
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().AddDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingCanceled().AddDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}
}

void UMainUIRoot::HandleMatchmakingStarted()
{
	PushLoadingOverlay();
}


void UMainUIRoot::HandleMatchmakingCanceled()
{
	PopLoadingOverlay();
}
