// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultPlayerController.h"
#include "EasyOnlineSession.h"
#include "Components/ActorComponents/LobbyCameraComponent.h"
#include "Framework/GameMode/InGameMode.h"
#include "Subsystems/ResultSceneSubsystem.h"

ADefaultPlayerController::ADefaultPlayerController()
{
	LobbyCameraComponent = CreateDefaultSubobject<ULobbyCameraComponent>(TEXT("LobbyCamera"));
}

void ADefaultPlayerController::AutoManageActiveCameraTarget(AActor* SuggestedTarget)
{
	if (LobbyCameraComponent)
	{
		if (AActor* ViewTargetOverride = LobbyCameraComponent->GetViewTargetOverride())
		{
			SuggestedTarget = ViewTargetOverride;
		}
	}

	Super::AutoManageActiveCameraTarget(SuggestedTarget);
}

void ADefaultPlayerController::ClientSetViewTarget_Implementation(AActor* A, FViewTargetTransitionParams TransitionParams)
{
	if (LobbyCameraComponent)
	{
		if (AActor* ViewTargetOverride = LobbyCameraComponent->GetViewTargetOverride())
		{
			Super::ClientSetViewTarget_Implementation(ViewTargetOverride, FViewTargetTransitionParams());
			return;
		}
	}

	Super::ClientSetViewTarget_Implementation(A, TransitionParams);
}

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
	if (UResultSceneSubsystem* ResultSubsystem = GetGameInstance()->GetSubsystem<UResultSceneSubsystem>())
	{
		ResultSubsystem->SaveResultSceneData();
	}

	Server_ReportClientTravelToResultLevelAndLeaveSession();

	if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
	{
		OnlineSession->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateLambda([this](FName /*SessionName*/, bool /*bSuccess*/)
		{
			if (const UResultSceneSubsystem* ResultSubsystem = GetGameInstance()->GetSubsystem<UResultSceneSubsystem>())
			{
				ResultSubsystem->OpenResultLevel(this);
			}
		}));
	}
}