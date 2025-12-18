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
#include "Actors/Rhythm/JudgementRing.h"

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
}

void ARhythmNoteSpawner::OnAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
	{
		const FString CueName = MusicInfo->UserCueName;
		SpawnAndMoveNote(CueName);
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
		CachedActorPoolSubsystem->Prewarm(RhythmNoteClass, 10, SpawnTransform);
		CachedActorPoolSubsystem->Prewarm(JudgementRingClass, 10, SpawnTransform);
	}
	if (ARhythmActor* OwnerActor = Cast<ARhythmActor>(GetOwner()))
	{
		CachedRhythmActor = OwnerActor;
	}
}

void ARhythmNoteSpawner::SpawnAndMoveNote(const FString& InUserCueName)
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
		
		PooledNote->InitNote(CachedRhythmActor.Get(), this, JudgementRingClass, TimeToComplete, InUserCueName);

		if (InUserCueName.StartsWith(TEXT("SS_")))
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
