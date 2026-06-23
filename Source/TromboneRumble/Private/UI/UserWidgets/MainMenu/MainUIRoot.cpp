// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "EasyMatchmakingManager.h"
#include "Utilities/TromboneStatics.h"

void UMainUIRoot::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().AddDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingComplete().AddDynamic(this, &ThisClass::HandleMatchmakingComplete);
		MatchmakingManager->OnMatchmakingCanceled().AddDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}
}

void UMainUIRoot::NativeDestruct()
{
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().RemoveDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingComplete().RemoveDynamic(this, &ThisClass::HandleMatchmakingComplete);
		MatchmakingManager->OnMatchmakingCanceled().RemoveDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}
	
	Super::NativeDestruct();
}

void UMainUIRoot::HandleMatchmakingStarted()
{
	UTromboneStatics::ShowLoadingOverlay(GetOwningPlayer());
	SetBaseUIEnabled(false);
}

void UMainUIRoot::HandleMatchmakingComplete(const FName SessionName, const EEasyMatchmakingCompleteResult Result)
{
	if (Result == EEasyMatchmakingCompleteResult::Failure || Result == EEasyMatchmakingCompleteResult::NoResults)
	{
		// TODO : 로컬라이징
		const FText ToastMessage = FText::FromString(TEXT("Matchmaking failed."));
		UTromboneStatics::ShowToast(GetWorld(), UTromboneStatics::MakeToastRequest(ToastMessage));
		
		UTromboneStatics::PopOverlay(GetOwningPlayer());
		SetBaseUIEnabled(true);
	}
}


void UMainUIRoot::HandleMatchmakingCanceled()
{
	UTromboneStatics::PopOverlay(GetOwningPlayer());
	SetBaseUIEnabled(true);
	
	// TODO : 로컬라이징
	const FText ToastMessage = FText::FromString(TEXT("Matchmaking canceled."));
	UTromboneStatics::ShowToast(GetWorld(), UTromboneStatics::MakeToastRequest(ToastMessage));
}
