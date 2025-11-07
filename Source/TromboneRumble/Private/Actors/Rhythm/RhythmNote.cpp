// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNote.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/ActorComponents/RhythmNoteUIControllerComponent.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmNoteWidget.h"
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

	RhythmNoteUIControllerComponent = CreateDefaultSubobject<URhythmNoteUIControllerComponent>(TEXT("RhythmNoteUIControllerComponent"));
}

void ARhythmNote::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	NoteLifeTime += DeltaTime;;
}

void ARhythmNote::OnTakenFromPool_Implementation()
{
	NoteLifeTime = 0.f;
	NoteAlphaOnSpline = 0.f;
	CachedSplineComponent = nullptr;

	NoteHandle = FNoteHandle();
	NoteHandle.NoteActor = this;

	CachedRhythmNoteChannelSubsystem->OpenChannel(NoteHandle.Id);
}


void ARhythmNote::OnReturnToPool_Implementation()
{
	NoteLifeTime = 0.f;
	NoteAlphaOnSpline = 1.f;
	CachedRhythmNoteChannelSubsystem->EmitDespawn(NoteHandle.Id);
	CachedRhythmNoteChannelSubsystem->CloseChannel(NoteHandle.Id);
}

void ARhythmNote::InitNote(const ARhythmNoteSpawner* InSpawner,URhythmNoteWidget* InNoteWidget, float InTimeToComplete,
	int32 InLineNum)
{
	checkf(InSpawner, TEXT("Spawner not Valid in %s"), *GetName());
	checkf(InSpawner->GetSpawnerType() != EInstrumentType::Invalid, TEXT("Spawner Type is Invalid"));
	checkf(InNoteWidget, TEXT("InNoteWidget not valid in %s"), *GetName());

	NoteType = InSpawner->GetSpawnerType();
	CachedSplineComponent = InSpawner->GetSplineComponent();
	TimeToComplete = InTimeToComplete;

	if (RhythmNoteUIControllerComponent)
	{
		RhythmNoteUIControllerComponent->InitSettings(InNoteWidget, NoteHandle, InLineNum);
	}
}

void ARhythmNote::SetToShortNote()
{
	bIsLongNote = false;
	bIsLongNoteEnd = false;
}

void ARhythmNote::SetToLongNoteStart()
{
	bIsLongNote = true;
	bIsLongNoteEnd = false;
}

void ARhythmNote::SetToLongNoteEnd()
{
	bIsLongNote = true;
	bIsLongNoteEnd = true;
}

void ARhythmNote::MoveNotes_Implementation()
{
	CachedRhythmNoteChannelSubsystem->UpdateProgress(NoteHandle.Id, NoteAlphaOnSpline);
}

void ARhythmNote::BeginPlay()
{
	Super::BeginPlay();
	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		CachedRhythmNoteChannelSubsystem = RhythmNoteChannelSubsystem;
	}
}





