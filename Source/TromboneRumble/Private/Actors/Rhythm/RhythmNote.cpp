// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNote.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "Utilities/DebugHelper.h"

ARhythmNote::ARhythmNote()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	OuterSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OuterSphere"));
	OuterSphere->SetupAttachment(RootComponent);
	MiddleSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MiddleSphere"));
	MiddleSphere->SetupAttachment(RootComponent);
	InnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InnerSphere"));
	InnerSphere->SetupAttachment(RootComponent);
}

void ARhythmNote::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	NoteLifeTime += DeltaTime;;
}

void ARhythmNote::OnTakenFromPool_Implementation()
{
	NoteLifeTime = 0.f;
	CachedSpawner = nullptr;
	CachedSplineComponent = nullptr;
}


void ARhythmNote::OnReturnToPool_Implementation()
{
	NoteLifeTime = 0.f;
}


void ARhythmNote::InitNote(ARhythmNoteSpawner* InSpawner, float InTimeToComplete, bool InIsLongNote, bool InIsLongNoteEnd)
{
	checkf(InSpawner, TEXT("Spawner not Valid"));
	CachedSpawner = InSpawner;
	CachedSplineComponent = InSpawner->GetSplineComponent();
	TimeToComplete = InTimeToComplete;
	bIsLongNote = InIsLongNote;
	bIsLongNoteEnd = InIsLongNoteEnd;
}

void ARhythmNote::MoveNotes_Implementation()
{
	if (!CachedSpawner.Get() || !CachedSplineComponent.Get())
	{
		return;
	}
}

void ARhythmNote::BeginPlay()
{
	Super::BeginPlay();
}





