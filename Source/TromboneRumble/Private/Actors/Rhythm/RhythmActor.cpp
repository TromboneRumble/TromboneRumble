// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmActor.h"

#include "Actors/Rhythm/RhythmNote.h"
#include "Components/BoxComponent.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
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
	RhythmNoteDestroyer->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnRhythmDestroyBeginOverlap);
}

void ARhythmActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARhythmActor::DetectNotes()
{
	TMap<ARhythmNote*, TSet<UPrimitiveComponent*>> NoteToHitComps;
	ARhythmNote* BestNote = GetBestNoteFromLineTrace(NoteToHitComps);
	if (!BestNote)
	{
		FString EnumName = StaticEnum<ENoteResult>()->GetNameStringByValue(static_cast<int64>(ENoteResult::Bad));
		Debug::Print(EnumName);
		return;
	}

	if (BestNote->IsLongNote() && !BestNote->IsLongNoteEnd())
	{
		//TODO : 롱노트 세부판정
		IsSensingLongNote = true;
		Debug::Print(TEXT("Long Note Sense Start"));
	}
	else
	{
		FRhythmTraceResult Result = ReturnNoteResult(BestNote, NoteToHitComps);
		if (Result.NoteActor.Get())
		{
			FString EnumName = StaticEnum<ENoteResult>()->GetNameStringByValue(static_cast<int64>(Result.Judge));
			Debug::Print(EnumName);
		}
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

ARhythmNoteSpawner* ARhythmActor::GetOrCreateSpawner(EInstrumentType InType)
{
	if (auto Found = RhythmNoteSpawners.Find(InType))
	{
		return Found->Get();
	}
	checkf(RhythmNoteSpawnerClass, TEXT("RhythmNoteSpawnerClass is null"));

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARhythmNoteSpawner* NewSpawner = World->SpawnActor<ARhythmNoteSpawner>(RhythmNoteSpawnerClass, GetActorTransform(), Params);
	if (!NewSpawner) return nullptr;

	if (USceneComponent* Root = GetRootComponent())
	{
		NewSpawner->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	}
	else
	{
		NewSpawner->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	}

	NewSpawner->SetActorRelativeLocation(FVector::ZeroVector);
	NewSpawner->SetActorRelativeRotation(FRotator::ZeroRotator);

	//Map에서 관리
	RhythmNoteSpawners.Add(InType, NewSpawner);

	NewSpawner->SpawnerType = InType;

	return NewSpawner;
}

bool ARhythmActor::DestroySpawner(EInstrumentType InType)
{
	if (auto Found = RhythmNoteSpawners.Find(InType))
	{
		ARhythmNoteSpawner* Spawner = Found->Get();
		if (IsValid(Spawner))
		{
			Spawner->Destroy();
		}
		RhythmNoteSpawners.Remove(InType);
		return true;
	}
	return false;
}


void ARhythmActor::BeginPlay()
{
	Super::BeginPlay();
	if (RhythmUIRootWidgetClass)
	{
		CachedRhythmUIRootWidget = CreateWidget<URhythmUIRootWidget>(GetWorld(), RhythmUIRootWidgetClass);
		if (CachedRhythmUIRootWidget)
		{
			CachedRhythmUIRootWidget->AddToViewport();
		}
	}
	//Todo : 음악에 맞게 스포너 생성
	GetOrCreateSpawner(EInstrumentType::Trombone);
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

	/*Debug::Print(FString::Printf(TEXT("Note Detected: %s, HitCount=%d, Judge=%d"),
		*InNote->GetName(), HitCount, static_cast<uint8>(RhythmResult.Judge)));*/

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
			const double T = Note->NoteLifeTime;
			if (T > BestTime) // 가장 큰 시간 = 가장 먼저 나온 노트
			{
				BestTime = T;
				Result = const_cast<ARhythmNote*>(Note);
			}
		}
	}
	return Result;
	
}

UActorPoolSubsystem* ARhythmActor::GetCachedSubsystem()
{
	if (CachedActorPoolSubsystem.IsValid())
		return CachedActorPoolSubsystem.Get();

	if (UActorPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
	{
		CachedActorPoolSubsystem = PoolSubsystem;
		return PoolSubsystem;
	}

	return nullptr;
}

void ARhythmActor::OnRhythmDestroyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
	{
		CachedActorPoolSubsystem->Release(OtherActor);
	}
}


