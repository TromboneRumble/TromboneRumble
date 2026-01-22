// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultPlayerController.h"
#include "Framework/InGameMode.h"
#include "Utilities/DebugHelper.h"

void ADefaultPlayerController::Server_RhythmGameFinished_Implementation()
{
	if (AInGameMode* GM = Cast<AInGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnRhythmGameEndedReport();
	}
}
