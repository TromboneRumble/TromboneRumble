// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RhythmSubsystem.h"
#include "AkGameplayTypes.h"
#include "Framework/InGameMode.h"

void URhythmSubsystem::OnMusicAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (CallbackType != EAkCallbackType::MusicSyncUserCue || !CallbackInfo)
	{
		return;
	}

	if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
	{
		const FString& CueString = MusicInfo->UserCueName;
		if (!CueString.IsEmpty())
		{
			const FName CueName(*CueString);
			BroadcastUserCue(CueName);
		}
	}
}

void URhythmSubsystem::OnMusicEndCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (CallbackType == EAkCallbackType::EndOfEvent)
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			if (AInGameMode* Gm = Cast<AInGameMode>(GetWorld()->GetAuthGameMode()))
			{
				FTimerHandle TimerHandle;
				const float Delay = 2.0f;
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, Gm]
				{
					Gm->GameEnd();
				}), Delay, false);
			}
		});
	}
}


void URhythmSubsystem::BroadcastUserCue(const FName& CueName)
{
	OnMusicUserCue.Broadcast(CueName);
}
