// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNote.h"
#include "Components/SphereComponent.h"
#include "Components/ActorComponents/RhythmNoteUIControllerComponent.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetBase.h"
#include "Utilities/DebugHelper.h"

ARhythmNote::ARhythmNote()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	OuterSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OuterSphere"));
	OuterSphere->SetupAttachment(RootComponent);
	InnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InnerSphere"));
	InnerSphere->SetupAttachment(RootComponent);

	RhythmNoteUIControllerComponent = CreateDefaultSubobject<URhythmNoteUIControllerComponent>(TEXT("RhythmNoteUIControllerComponent"));
}

void ARhythmNote::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsMoving) return;
	NoteLifeTime += DeltaTime;
	float Alpha = FMath::Clamp(NoteLifeTime / TimeToComplete, 0.f, 1.f);
	FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	SetActorLocation(NewLocation);
	CachedRhythmNoteChannelSubsystem->UpdateProgress(NoteHandle.Id, Alpha);
}

void ARhythmNote::OnTakenFromPool_Implementation()
{
	NoteLifeTime = 0.f;
	bIsMoving = true;

	NoteHandle = FNoteHandle();
	NoteHandle.NoteActor = this;

	CachedRhythmNoteChannelSubsystem->OpenChannel(NoteHandle.Id);
}


void ARhythmNote::OnReturnToPool_Implementation()
{
	NoteLifeTime = 0.f;
	bIsMoving = false;

	CachedRhythmNoteChannelSubsystem->EmitDespawn(NoteHandle.Id);
	CachedRhythmNoteChannelSubsystem->CloseChannel(NoteHandle.Id);
}

void ARhythmNote::InitNote(const ARhythmNoteSpawner* InSpawner,URhythmNoteWidgetBase* InNoteWidget, float InTimeToComplete)
{
	checkf(InSpawner, TEXT("Spawner not Valid in %s"), *GetName());
	checkf(InSpawner->GetSpawnerType() != EInstrumentType::Invalid, TEXT("Spawner Type is Invalid"));
	checkf(InNoteWidget, TEXT("InNoteWidget not valid in %s"), *GetName());

	NoteType = InSpawner->GetSpawnerType();
	TimeToComplete = InTimeToComplete;

	StartLocation = InSpawner->GetActorLocation();
	EndLocation = StartLocation + FVector(1000.f, 0.f, 0.f);

	if (RhythmNoteUIControllerComponent)
	{
		RhythmNoteUIControllerComponent->InitSettings(InSpawner->GetSpawnWidget(), InNoteWidget, NoteHandle);
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
	bIsMoving = true;
}

void ARhythmNote::SpawnRhythmResultWidget(ENoteResult InNoteResult)
{
	if (RhythmNoteUIControllerComponent)
	{
		RhythmNoteUIControllerComponent->SpawnRhythmResultWidget(InNoteResult);
	}
}

void ARhythmNote::BeginPlay()
{
	Super::BeginPlay();
	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		CachedRhythmNoteChannelSubsystem = RhythmNoteChannelSubsystem;
	}
}





