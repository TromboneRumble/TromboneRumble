// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "Components/ArrowComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Actors/Rhythm/RhythmNote.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetBase.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"
#include "Actors/Rhythm/NoteVisualizer.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/API/WwiseMusicEngineAPI.h"

ARhythmNoteSpawner::ARhythmNoteSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("NoteSpawner"));

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->SetupAttachment(RootComponent);
		ArrowComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	}
}

void ARhythmNoteSpawner::InitSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent, UAkSwitchValue* InChangeSwitch,
	UAkAudioEvent* InFailEvent, bool InIsSyncTesting)
{
	SpawnerType = InType;
	SpawnNoteEvent = InNoteEvent;
	ChangeSwitch = InChangeSwitch;
	FailEvent = InFailEvent;
	IsSyncTesting = InIsSyncTesting;
	if (NoteSpawnPlayingID != 0 && NoteSpawnPlayingID != AK_INVALID_PLAYING_ID)
	{
		if (auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Stop, NoteSpawnPlayingID);
		}
	}
	NoteSpawnPlayingID = 0;
	ResetMusicClock();
}

double ARhythmNoteSpawner::GetMusicTimeSeconds()
{
	if (GFrameCounter == LastClockQueryFrame)
	{
		return MusicClockSec;
	}
	LastClockQueryFrame = GFrameCounter;

	// 일시정지 중에는 외삽이 계속 앞서 나가므로 클럭을 얼려둔다
	if (bClockPaused)
	{
		return MusicClockSec;
	}

	bool bAdvanced = false;
	if (NoteSpawnPlayingID != 0 && NoteSpawnPlayingID != AK_INVALID_PLAYING_ID)
	{
		if (IWwiseMusicEngineAPI* MusicEngine = IWwiseMusicEngineAPI::Get())
		{
			AkSegmentInfo SegmentInfo;
			// "nothing" 구간의 0-채움, pre-entry 음수, 역행은 버리고 전진만 채택한다
			if (MusicEngine->GetPlayingSegmentInfo(static_cast<AkPlayingID>(NoteSpawnPlayingID), SegmentInfo, true) == AK_Success
				&& SegmentInfo.iCurrentPosition > LastRawPositionMs)
			{
				LastRawPositionMs = SegmentInfo.iCurrentPosition;
				MusicClockSec = SegmentInfo.iCurrentPosition / 1000.0;
				bMusicClockValid = true;
				ClockStallSeconds = 0.0;
				bAdvanced = true;
			}
		}
	}

	// 살아 있던 클럭이 오래 멈춰 있으면 무효로 내려 소비자를 DeltaTime 폴백으로 돌린다
	if (!bAdvanced && bMusicClockValid)
	{
		ClockStallSeconds += GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0;
		if (ClockStallSeconds > 0.5)
		{
			bMusicClockValid = false;
			UE_LOG(LogTemp, Warning, TEXT("RhythmNoteSpawner(%s) 음악 클럭 0.5초 스톨 - DeltaTime 폴백 전환"), *GetNameSafe(this));
		}
	}

	return MusicClockSec;
}

void ARhythmNoteSpawner::ResetMusicClock()
{
	MusicClockSec = 0.0;
	LastRawPositionMs = 0;
	bMusicClockValid = false;
	bClockPaused = false;
	LastClockQueryFrame = 0;
	ClockStallSeconds = 0.0;
}

void ARhythmNoteSpawner::OnAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
	{
		const FString CueName = MusicInfo->UserCueName;
		// 콜백에 실려온 위치가 이 노트의 정확한 스폰 시각이다. 0 이하면(0-채움/pre-entry) 클럭으로 폴백
		const double SpawnMusicTimeSec = (MusicInfo->SegmentInfo.CurrentPosition > 0)
			? MusicInfo->SegmentInfo.CurrentPosition / 1000.0
			: GetMusicTimeSeconds();
		SpawnAndMoveNote(CueName, SpawnMusicTimeSec);
	}

}

void ARhythmNoteSpawner::PauseRhythmGame()
{
	bClockPaused = true;

	if (NoteSpawnPlayingID != 0 && NoteSpawnPlayingID != AK_INVALID_PLAYING_ID)
	{
		if(auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Pause, NoteSpawnPlayingID);
		}
	}
	for (auto It = ActiveNotes.CreateIterator(); It; ++It)
	{
		if (ARhythmNote* Note = It->Get())
		{
			Note->SetPause(true);
		}
	}
}

void ARhythmNoteSpawner::ResumeRhythmGame()
{
	bClockPaused = false;

	if (NoteSpawnPlayingID != 0 && NoteSpawnPlayingID != AK_INVALID_PLAYING_ID)
	{
		if (auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Resume, NoteSpawnPlayingID);
		}
	}
	for (auto It = ActiveNotes.CreateIterator(); It; ++It)
	{
		if (ARhythmNote* Note = It->Get())
		{
			Note->SetPause(false);
		}
	}
}

void ARhythmNoteSpawner::StopRhythmGame()
{
	if (NoteSpawnPlayingID != 0 && NoteSpawnPlayingID != AK_INVALID_PLAYING_ID)
	{
		if (auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Stop, NoteSpawnPlayingID);
		}
		NoteSpawnPlayingID = 0;
	}
	ResetMusicClock();


	if (UActorPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
	{
		
		for (auto It = ActiveNotes.CreateIterator(); It; ++It)
		{
			if (ARhythmNote* Note = It->Get())
			{
				PoolSubsystem->Release(Note);
			}
		}
	}
	ActiveNotes.Empty();
}

void ARhythmNoteSpawner::RemoveActiveNote(ARhythmNote* Note)
{
	if (Note)
	{
		ActiveNotes.Remove(Note);
	}
}

void ARhythmNoteSpawner::BeginPlay()
{
	Super::BeginPlay();
	CachedActorPoolSubsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>();
	CachedRhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>();
	if (CachedActorPoolSubsystem.Get() && RhythmNoteClass)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation());
		SpawnTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
		SpawnTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
		CachedActorPoolSubsystem->Prewarm(RhythmNoteClass, PoolPrewarmCount, SpawnTransform);
		CachedActorPoolSubsystem->Prewarm(NoteVisualizerClass, PoolPrewarmCount, SpawnTransform);
	}
	if (ARhythmActor* OwnerActor = Cast<ARhythmActor>(GetOwner()))
	{
		CachedRhythmActor = OwnerActor;
	}
}

void ARhythmNoteSpawner::SpawnAndMoveNote(const FString& InUserCueName, double InSpawnMusicTimeSec)
{
	checkf(RhythmNoteClass, TEXT("RhythmNoteClass is not set in %s"), *GetName());
	if (!CachedRhythmNoteChannelSubsystem.Get() || !CachedActorPoolSubsystem.Get()) return;

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(GetActorLocation());
	SpawnTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
	SpawnTransform.SetScale3D(FVector(1.f, 1.f, 1.f));

	//숏노트만 나오게 임시로 설정
	if (!InUserCueName.StartsWith(TEXT("SS_")))
	{
		return;
	}

	if (ARhythmNote* PooledNote = Cast<ARhythmNote>(CachedActorPoolSubsystem->Acquire(RhythmNoteClass, SpawnTransform)))
	{
		
		PooledNote->InitNote(CachedRhythmActor.Get(), this, NoteVisualizerClass, TimeToComplete, InUserCueName, InSpawnMusicTimeSec);
		ActiveNotes.Add(PooledNote);

		bool bIsN = InUserCueName.Equals(TEXT("N"), ESearchCase::IgnoreCase);
		bool bIsShortNotePrefix = InUserCueName.StartsWith(TEXT("SS_"));

		if (bIsN || bIsShortNotePrefix)
		{
			PooledNote->SetToShortNote();
		}
		else if (InUserCueName.StartsWith(TEXT("LS_")))
		{
			PooledNote->SetToLongNoteStart();
		}
		else if (InUserCueName.StartsWith(TEXT("LC_")))
		{
			//TODO : Lone Note Change 만들기
			return;
		}
		else if (InUserCueName.StartsWith(TEXT("LE_")))
		{
			PooledNote->SetToLongNoteEnd();
		}
		else
		{
			checkf(false, TEXT("Unknown CueName: %s"), *InUserCueName);
		}

		PooledNote->MoveNotes();

		//싱크가 맞는지 확인하는 디버그 코드
		if (IsSyncTesting)
		{
			float Delay = TimeToComplete;
			if (ARhythmActor* OwnerRhythmActor = Cast<ARhythmActor>(GetOwner()))
			{
				PooledNote->StartSyncDebugTimer(OwnerRhythmActor, Delay);
			}
		}
		
	}
	
}
