// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RhythmSubsystem.h"
#include "AkGameplayTypes.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Kismet/GameplayStatics.h"

void URhythmSubsystem::PauseRhythmGame()
{
	if (RhythmActor)
	{
		RhythmActor->PauseRhythmGame();
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Paused);
	}
}

void URhythmSubsystem::ResumeRhythmGame()
{
	if (RhythmActor)
	{
		RhythmActor->ResumeRhythmGame();
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Resumed);
	}
}

void URhythmSubsystem::StopRhythmGame()
{
	if (RhythmActor)
	{
		//TODO : StopRhythmGame구현
		//RhythmActor->StopRhythmGame();
		//OnRhythmGameStateChanged.Broadcast(ERhythmGameState::End);
	}
}

void URhythmSubsystem::HandleMusicCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	
	if (CallbackType == EAkCallbackType::MusicSyncUserCue)
	{
		OnMusicAkCallback(CallbackType, CallbackInfo);
	}
	else if (CallbackType == EAkCallbackType::EndOfEvent)
	{
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Ended);
	}
}

void URhythmSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (UWorld* World = GetWorld())
	{
		World->OnWorldBeginPlay.AddUObject(this, &ThisClass::OnWorldBeginPlay);
	}
}

void URhythmSubsystem::OnWorldBeginPlay()
{
	if (UWorld* World = GetWorld())
	{
		if (AActor* FoundActor = UGameplayStatics::GetActorOfClass(World, ARhythmActor::StaticClass()))
		{
			RhythmActor = Cast<ARhythmActor>(FoundActor);
		}
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
