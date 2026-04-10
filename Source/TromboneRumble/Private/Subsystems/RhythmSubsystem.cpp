// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RhythmSubsystem.h"
#include "AkGameplayTypes.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Framework/TromboneGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"

void URhythmSubsystem::StartRhythmGame(const FGameplayTag& InGamePlayTag)
{
	if (RhythmActor.Get())
	{
		RhythmActor->PrepareAndStartRhythmGame(InGamePlayTag);
		CurrentState = ERhythmGameState::Start;
		isRhythmGameForceStopped = false;
		//Broadcast는 RhythmActor에서 노래 준비가 다 끝난후에 호출
		//OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Start);
	}
	else
	{
		Debug::Print(TEXT("RhythmSubsystem StartRhythmGame : No Rhythm Actor Found"));
	}
}

void URhythmSubsystem::PauseRhythmGame()
{
	if (RhythmActor.Get())
	{
		RhythmActor->PauseRhythmGame();
		CurrentState = ERhythmGameState::Paused;
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Paused);
	}
	else
	{
		Debug::Print(TEXT("RhythmSubsystem PauseRhythmGame: No Rhythm Actor Found"));
	}
}

void URhythmSubsystem::ResumeRhythmGame()
{
	if (RhythmActor.Get())
	{
		RhythmActor->ResumeRhythmGame();
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Resumed);
		CurrentState = ERhythmGameState::Playing;
	}
	else
	{
		Debug::Print(TEXT("RhythmSubsystem ResumeRhythmGame : No Rhythm Actor Found"));
	}
}

void URhythmSubsystem::StopRhythmGame()
{
	if (RhythmActor.Get())
	{
		RhythmActor->StopRhythmGame();
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Stopped);
		CurrentState = ERhythmGameState::Stopped;
		isRhythmGameForceStopped = true;
	}
	else
	{
		Debug::Print(TEXT("RhythmSubsystem StopRhythmGame : No Rhythm Actor Found"));
	}
}

void URhythmSubsystem::EndRhythmGame()
{
	if (RhythmActor.Get())
	{
		RhythmActor->StopRhythmGame();
		OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Ended);
		CurrentState = ERhythmGameState::Ended;
	}
	else
	{
		Debug::Print(TEXT("RhythmSubsystem EndRhythmGame : No Rhythm Actor Found"));
	}
}

void URhythmSubsystem::HandleMusicCallbacksFromRhythmActor(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	OnMusicCallback.Broadcast(CallbackType, CallbackInfo);

	switch (CallbackType)
	{
	case EAkCallbackType::EndOfEvent:
		if (!isRhythmGameForceStopped)
		{
			RhythmActor->StopRhythmGame();
			OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Ended);
			CurrentState = ERhythmGameState::Ended;
		}
		break;
	case EAkCallbackType::Marker:
		break;
	case EAkCallbackType::Duration:
		break;
	case EAkCallbackType::Starvation:
		break;
	case EAkCallbackType::MusicPlayStarted:
		break;
	case EAkCallbackType::MusicSyncBeat:
		break;
	case EAkCallbackType::MusicSyncBar:
		break;
	case EAkCallbackType::MusicSyncEntry:
		break;
	case EAkCallbackType::MusicSyncExit:
		break;
	case EAkCallbackType::MusicSyncGrid:
		break;
	case EAkCallbackType::MusicSyncUserCue:
		break;
	case EAkCallbackType::MusicSyncPoint:
		break;
	case EAkCallbackType::MIDIEvent:
		break;
	}
}

void URhythmSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URhythmSubsystem::RegisterRhythmActor(ARhythmActor* InActor)
{
	RhythmActor = InActor;
}
