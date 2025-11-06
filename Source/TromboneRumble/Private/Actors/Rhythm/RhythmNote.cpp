// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNote.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/ActorComponents/RhythmNoteUIControllerComponent.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
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
	CachedSplineComponent = nullptr;
	if (CreatedWidget)
	{
		CreatedWidget->SetVisibility(ESlateVisibility::Visible);
	}
}


void ARhythmNote::OnReturnToPool_Implementation()
{
	NoteLifeTime = 0.f;
	if (CreatedWidget)
	{
		CreatedWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}


void ARhythmNote::InitNote(const ARhythmNoteSpawner* InSpawner, URhythmNoteWidget* InWidget, float InTimeToComplete, int32 LineNum)
{
	checkf(InSpawner, TEXT("Spawner not Valid in %s"), *GetName());
	checkf(InSpawner->GetSpawnerType() != EInstrumentType::Invalid, TEXT("Spawner Type is Invalid"));
	checkf(InWidget, TEXT("InWidget not valid in %s"), *GetName());

	NoteType = InSpawner->GetSpawnerType();
	CachedSplineComponent = InSpawner->GetSplineComponent();
	TimeToComplete = InTimeToComplete;

	if (RhythmNoteUIControllerComponent)
	{
		RhythmNoteUIControllerComponent->InitSettings(InSpawner->GetSpawnWidget(), InWidget, LineNum);
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
	if (!CachedSplineComponent.Get())
	{
		return;
	}
}

void ARhythmNote::BeginPlay()
{
	Super::BeginPlay();
	checkf(RhythmNoteWidgetClass, TEXT("RhythmNoteWidgetClass is not set in %s"), *GetName());
	if (CreatedWidget) return;
	UWorld* World = GetWorld();
	if (World)
	{
		// 위젯 생성
		CreatedWidget = CreateWidget<URhythmNoteWidget>(World, RhythmNoteWidgetClass);
		if (CreatedWidget)
		{
			CreatedWidget->AddToViewport();
			

			// 또는 특정 Parent Widget에 AddChild 할 수도 있음
			UE_LOG(LogTemp, Log, TEXT("Widget created and added to viewport"));
		}
	}
}





