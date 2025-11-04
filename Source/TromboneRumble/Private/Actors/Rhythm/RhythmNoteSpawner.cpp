// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"
#include "Actors/Rhythm/RhythmNote.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmSpawnWidget.h"
#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
#include "Utilities/DebugHelper.h"

ARhythmNoteSpawner::ARhythmNoteSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("NoteSpawner"));
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Lane"));
	if (SplineComponent)
	{
		SplineComponent->SetupAttachment(RootComponent);
		SplineComponent->SetLocationAtSplinePoint(
			1, FVector(1500.f, 0.f, 0.f),
			ESplineCoordinateSpace::Local,
			true);
	}
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->SetupAttachment(RootComponent);
		ArrowComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	}

	

}

void ARhythmNoteSpawner::InitSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent, UAkSwitchValue* InChangeSwitch,
	UAkAudioEvent* InFailEvent)
{
	SpawnerType = InType;
	SpawnNoteEvent = InNoteEvent;
	ChangeSwitch = InChangeSwitch;
	FailEvent = InFailEvent;
}

void ARhythmNoteSpawner::BeginPlay()
{
	Super::BeginPlay();

	UActorPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>();
	if (PoolSubsystem && RhythmNoteClass)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation());
		SpawnTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
		SpawnTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
		PoolSubsystem->Prewarm(RhythmNoteClass, 100, SpawnTransform);
	}
	OwnerRhythmActor = Cast<ARhythmActor>(GetOwner());
	if (IsValid(OwnerRhythmActor))
	{
		SpawnWidget = CreateWidget<URhythmSpawnWidget>(GetWorld(), RhythmSpawnWidgetClass);
		if (SpawnWidget)
		{
			UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(OwnerRhythmActor->GetRhythmUIRootWidget()->NoteCanvas->AddChild(SpawnWidget));
			NoteSlot->SetAutoSize(true);
			NoteSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			NoteSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			//TODO : Remove Magic Number
			NoteSlot->SetPosition(FVector2D(175.f, -300.f));
		}
	}

}

void ARhythmNoteSpawner::OnAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
	{
		const FString CueName = MusicInfo->UserCueName;
		SpawnRhythmNote(5.f, false, false);
		if (CueName.StartsWith(TEXT("SS_")))
		{
			
		}
		
	}
	
}

void ARhythmNoteSpawner::SpawnRhythmNote(float TimeToComplete, bool InIsLongNote, bool InIsLongNoteEnd)
{
	checkf(RhythmNoteClass, TEXT("RhythmNoteClass is not set in %s"), *GetName());
	if (UActorPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation());
		SpawnTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
		SpawnTransform.SetScale3D(FVector(1.f, 1.f, 1.f));

		if (ARhythmNote* PooledNote = Cast<ARhythmNote>(PoolSubsystem->Acquire(RhythmNoteClass, SpawnTransform)))
		{
			PooledNote->InitNote(this, TimeToComplete,InIsLongNote,InIsLongNoteEnd);
			PooledNote->MoveNotes();
		}
	}
}