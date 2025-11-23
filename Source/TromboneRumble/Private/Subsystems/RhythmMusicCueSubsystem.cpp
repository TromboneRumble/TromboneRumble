// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/RhythmMusicCueSubsystem.h"
#include "AkGameplayTypes.h"
#include "Utilities/DebugHelper.h"

void URhythmMusicCueSubsystem::OnMusicAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
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


void URhythmMusicCueSubsystem::BroadcastUserCue(const FName& CueName)
{
	OnMusicUserCue.Broadcast(CueName);
}
