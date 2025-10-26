// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNoteSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Actors/Rhythm/RhythmNote.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/ActorPoolSubsystem.h"

ARhythmNoteSpawner::ARhythmNoteSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("NoteSpawner"));
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Lane"));
	if (SplineComponent)
	{
		SplineComponent->SetupAttachment(RootComponent);
		SplineComponent->SetLocationAtSplinePoint(
			1, FVector(1000.f, 0.f, 0.f),
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
}

void ARhythmNoteSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARhythmNoteSpawner::SpawnRhythmNote(float TimeToComplete)
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
			PooledNote->InitNote(this, TimeToComplete);
			PooledNote->MoveNotes();
		}
	}
}

