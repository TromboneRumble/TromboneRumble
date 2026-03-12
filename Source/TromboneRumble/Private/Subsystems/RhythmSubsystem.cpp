// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RhythmSubsystem.h"
#include "AkGameplayTypes.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Kismet/GameplayStatics.h"


void URhythmSubsystem::StartRhythmGame()
{
	if (RhythmActor)
	{
		RhythmActor->PrepareAndStartRhythmGame();
		CurrentState = ERhythmGameState::Start;
		isRhythmGameForceStopped = false;
		//Broadcast는 RhythmActor에서 노래 준비가 다 끝난후에 호출
		//OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Start);
	}
}

void URhythmSubsystem::PauseRhythmGame()
{
	if (RhythmActor)
	{
		RhythmActor->PauseRhythmGame();
		CurrentState = ERhythmGameState::Paused;
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Paused);
	}
}

void URhythmSubsystem::ResumeRhythmGame()
{
	if (RhythmActor)
	{
		RhythmActor->ResumeRhythmGame();
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Resumed);
		CurrentState = ERhythmGameState::Playing;
	}
}

void URhythmSubsystem::StopRhythmGame()
{
	if (RhythmActor)
	{
		RhythmActor->StopRhythmGame();
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Stopped);
		CurrentState = ERhythmGameState::Stopped;
		isRhythmGameForceStopped = true;
	}
}

void URhythmSubsystem::EndRhythmGame()
{
	if (RhythmActor)
	{
		RhythmActor->StopRhythmGame();
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Ended);
		CurrentState = ERhythmGameState::Ended;
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
		if (!isRhythmGameForceStopped)
		{
			RhythmActor->StopRhythmGame();
			OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Ended);
			CurrentState = ERhythmGameState::Ended;
		}
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
