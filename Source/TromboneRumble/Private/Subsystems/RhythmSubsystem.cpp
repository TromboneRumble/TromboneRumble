// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RhythmSubsystem.h"
#include "AkGameplayTypes.h"
#include "GameFramework/PlayerState.h"
#include "Utilities/DebugHelper.h"

void URhythmSubsystem::HandleMusicCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	
	if (CallbackType == EAkCallbackType::MusicSyncUserCue)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("MusicSyncUserCue"));
		OnMusicAkCallback(CallbackType, CallbackInfo);
	}
	else if (CallbackType == EAkCallbackType::EndOfEvent)
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			OnRhythmGameEnded.Broadcast();
		});
	}
}

void URhythmSubsystem::OnMusicAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (CallbackType != EAkCallbackType::MusicSyncUserCue || !CallbackInfo) return;

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

void URhythmSubsystem::BroadcastUserCue(const FName& CueName)
{
	OnMusicUserCue.Broadcast(CueName);
}
