// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmActor.h"

#include "Actors/Rhythm/RhythmNote.h"
#include "Components/BoxComponent.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "Utilities/Defines.h"
#include "Utilities/DebugHelper.h"



ARhythmActor::ARhythmActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	TraceStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	TraceStartPoint->SetupAttachment(GetRootComponent());
	TraceEndPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	TraceEndPoint->SetupAttachment(GetRootComponent());

	RhythmNoteDestroyer = CreateDefaultSubobject<UBoxComponent>(TEXT("Note Destroyer"));
	RhythmNoteDestroyer->SetupAttachment(GetRootComponent());

	RhythmNoteSpawner = CreateDefaultSubobject<UChildActorComponent>(TEXT("Note Spawner"));
	RhythmNoteSpawner->SetupAttachment(GetRootComponent());
	RhythmNoteSpawner->SetRelativeLocation(FVector::ZeroVector);
}

void ARhythmActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARhythmActor::DetectNotes()
{
	UWorld* World = GetWorld();
	if (!World) return;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	
	TArray<FHitResult> OutHits;
	const bool bHit = World->LineTraceMultiByChannel(OutHits,
		TraceStartPoint->GetComponentLocation(),
		TraceEndPoint->GetComponentLocation(),
		ECollisionChannel::ECC_GameTraceChannel2,
		params);
	Debug::Print(FString::Printf(TEXT("Hit Detected: %d"), OutHits.Num()));

	if (OutHits.Num() == 0) return;

	

	//노트별로 히트된 컴포넌트 수
	TMap<ARhythmNote*, TSet<UPrimitiveComponent*>> NoteToHitComps;
	for (const FHitResult& H : OutHits)
	{
		if (ARhythmNote* Note = Cast<ARhythmNote>(H.GetActor()))
		{
			if (UPrimitiveComponent* HitComp = H.GetComponent())
			{
				NoteToHitComps.FindOrAdd(Note).Add(HitComp);
			}
		}
	}
	if (NoteToHitComps.Num() == 0) return;

	//가장 먼저 나온 노드 찾기
	ARhythmNote* BestNote = nullptr;
	double BestTime = -DBL_MAX;

	for (const auto& Pair : NoteToHitComps)
	{
		if (const ARhythmNote* Note = Pair.Key)
		{
			const double T = Note->NoteTimeSec;
			if (T > BestTime) // 가장 큰 시간 = 가장 먼저 나온 노트
			{
				BestTime = T;
				BestNote = const_cast<ARhythmNote*>(Note);
			}
		}
	}

	FRhythmTraceResult RhythmResult;
	if (BestNote)
	{
		const int32 HitCount = NoteToHitComps.FindChecked(BestNote).Num();
		RhythmResult.NoteActor = BestNote;

		if (HitCount == 1)
		{
			RhythmResult.Judge = ENoteResult::Good;
		}
		else if (HitCount == 2)
		{
			RhythmResult.Judge = ENoteResult::Great;
		}
		else if (HitCount >= 3)
		{
			RhythmResult.Judge = ENoteResult::Excellent; // 또는 Perfect
		}
		else
		{
			RhythmResult.Judge = ENoteResult::Bad;
		}

		Debug::Print(FString::Printf(TEXT("Note Detected: %s, HitCount=%d, Judge=%d"),
			*BestNote->GetName(), HitCount, static_cast<uint8>(RhythmResult.Judge)));
	}
}



void ARhythmActor::BeginPlay()
{
	Super::BeginPlay();
	
}

