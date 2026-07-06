// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultPlayerController.h"
#include "EasyOnlineSession.h"
#include "Framework/TromboneGameInstance.h"
#include "Framework/GameMode/InGameMode.h"
#include "Utilities/TromboneStatics.h"

void ADefaultPlayerController::Server_RhythmGameFinished_Implementation()
{
	if (AInGameMode* GM = Cast<AInGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnRhythmGameEndedReport(this);
	}
}

void ADefaultPlayerController::Server_ReportClientTravelToResultLevelAndLeaveSession_Implementation()
{
	if (AInGameMode* Gm = GetWorld()->GetAuthGameMode<AInGameMode>())
	{
		Gm->OnClientTravelToResultLevelAndLeaveSession();
	}
}

void ADefaultPlayerController::Client_RequestTravelToResultLevelAndLeaveSession_Implementation()
{
	if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		GI->SaveResultSceneData();
	}
	
	Server_ReportClientTravelToResultLevelAndLeaveSession();

	if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
	{
		OnlineSession->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateLambda([this](FName /*SessionName*/, bool /*bSuccess*/)
		{
			UTromboneStatics::OpenLevel(this, ELevelType::ResultScene);
		}));
	}
}