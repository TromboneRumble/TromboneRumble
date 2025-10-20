// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNoteSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"

#include "Actors/Rhythm/RhythmNote.h"

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
	
	
}

void ARhythmNoteSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARhythmNoteSpawner::SpawnRhythmNote(float TimeToComplete)
{
	if (RhythmNoteClass)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation());
		SpawnTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
		SpawnTransform.SetScale3D(FVector(1.f, 1.f, 1.f));

		FActorSpawnParameters params;
		params.Owner = this;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ARhythmNote* Note = GetWorld()->SpawnActor<ARhythmNote>(
			RhythmNoteClass,
			SpawnTransform,
			params);
		if (Note)
		{
			Note->SetTimeToComplete(TimeToComplete);
		}
	}
}

