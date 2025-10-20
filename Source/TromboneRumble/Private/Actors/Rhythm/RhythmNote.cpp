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


void ARhythmNote::MoveNotes_Implementation()
{

}

void ARhythmNote::BeginPlay()
{
	Super::BeginPlay();
	if (ARhythmNoteSpawner* Spawner = Cast<ARhythmNoteSpawner>(GetOwner()))
	{
		CachedSpawner = Spawner;
		CachedSplineComponent = Spawner->GetSplineComponent();
	}
	else
	{
		CachedSpawner = nullptr;
		ensureMsgf(false, TEXT("Owner is not ARhythmNoteSpawner. Owner=%s"), *GetNameSafe(GetOwner()));
	}
}





