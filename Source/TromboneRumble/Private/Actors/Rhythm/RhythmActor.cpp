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
	TMap<ARhythmNote*, TSet<UPrimitiveComponent*>> NoteToHitComps;
	ARhythmNote* BestNote = GetBestNoteFromLineTrace(NoteToHitComps);
	if (!BestNote) return;
	if (BestNote->IsLongNote() && !BestNote->IsLongNoteEnd())
	{
		//TODO : 롱노트 세부판정
		IsSensingLongNote = true;
		Debug::Print(TEXT("Long Note Sense Start"));
	}
	else
	{
		ReturnNoteResult(BestNote, NoteToHitComps);
	}
}

void ARhythmActor::DetectLongNoteEnd()
{
	if (!IsSensingLongNote) return;
	Debug::Print(TEXT("Long Note Sense End"));
	IsSensingLongNote = false;
	TMap<ARhythmNote*, TSet<UPrimitiveComponent*>> NoteToHitComps;
	ARhythmNote* BestNote = GetBestNoteFromLineTrace(NoteToHitComps);
	if (!BestNote) return;
	if (BestNote->IsLongNote() && BestNote->IsLongNoteEnd())
	{
		ReturnNoteResult(BestNote, NoteToHitComps);
	}
	
}


void ARhythmActor::BeginPlay()
{
	Super::BeginPlay();

}


FRhythmTraceResult ARhythmActor::ReturnNoteResult(ARhythmNote* InNote, const TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InNoteToHitComps)
{

	FRhythmTraceResult RhythmResult;
	if (InNote == nullptr)
	{
		return RhythmResult;
	}
	const int32 HitCount = InNoteToHitComps.FindChecked(InNote).Num();
	RhythmResult.NoteActor = InNote;

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
		*InNote->GetName(), HitCount, static_cast<uint8>(RhythmResult.Judge)));

	return RhythmResult;
}



ARhythmNote* ARhythmActor::GetBestNoteFromLineTrace(TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InOutNoteToHitComps)
{
	ARhythmNote* Result = nullptr;

	UWorld* World = GetWorld();
	if (!World) return Result;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;
	const bool bHit = World->LineTraceMultiByChannel(HitResults,
		TraceStartPoint->GetComponentLocation(),
		TraceEndPoint->GetComponentLocation(),
		ECollisionChannel::ECC_GameTraceChannel2,
		params);
	Debug::Print(FString::Printf(TEXT("Hit Detected: %d"), HitResults.Num()));

	if (HitResults.Num() == 0) return Result;


	//노트별로 히트된 컴포넌트 수 집계
	
	for (const FHitResult& H : HitResults)
	{
		if (ARhythmNote* Note = Cast<ARhythmNote>(H.GetActor()))
		{
			if (UPrimitiveComponent* HitComp = H.GetComponent())
			{
				InOutNoteToHitComps.FindOrAdd(Note).Add(HitComp);
			}
		}
	}
	if (InOutNoteToHitComps.Num() == 0) return Result;

	//가장 먼저 나온 노드 찾기
	ARhythmNote* BestNote = nullptr;
	double BestTime = -DBL_MAX;

	for (const auto& Pair : InOutNoteToHitComps)
	{
		if (const ARhythmNote* Note = Pair.Key)
		{
			const double T = Note->NoteTimeSec;
			if (T > BestTime) // 가장 큰 시간 = 가장 먼저 나온 노트
			{
				BestTime = T;
				Result = const_cast<ARhythmNote*>(Note);
			}
		}
	}
	return Result;
	
}


