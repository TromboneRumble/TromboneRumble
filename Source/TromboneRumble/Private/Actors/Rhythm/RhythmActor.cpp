// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmActor.h"

#include "AkAudioEvent.h"
#include "Components/BoxComponent.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "AkSwitchValue.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "Actors/Rhythm/RhythmNote.h"
#include "Data/RhythmSongDataRow.h"
#include "Framework/TromboneGameInstance.h"
#include "Subsystems/GameDataSubsystem.h"
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
	

	NoteSpawnComponent = CreateDefaultSubobject<UAkComponent>(TEXT("NoteSpawnAKComponent"));
	if (NoteSpawnComponent)
	{
		NoteSpawnComponent->SetupAttachment(RootComponent);
		NoteSpawnComponent->OcclusionRefreshInterval = 0.f;
	}
	NoteHearingComponent = CreateDefaultSubobject<UAkComponent>(TEXT("NoteHearingAKComponent"));
	if (NoteHearingComponent)
	{
		NoteHearingComponent->SetupAttachment(RootComponent);
		NoteHearingComponent->OcclusionRefreshInterval = 0.f;
	}
}

void ARhythmActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

ENoteResult ARhythmActor::DetectNotes()
{
	if (FocusedType == EInstrumentType::Background || FocusedType == EInstrumentType::Invalid)
	{
		return ENoteResult::None;
	}
	TMap<ARhythmNote*, TSet<UPrimitiveComponent*>> NoteToHitComps;
	ARhythmNote* BestNote = GetBestNoteFromLineTrace(NoteToHitComps);
	if (!BestNote)
	{
		/*FString EnumName = StaticEnum<ENoteResult>()->GetNameStringByValue(static_cast<int64>(ENoteResult::Bad));
		Debug::Print(EnumName);*/
		OnNoteDetected.Broadcast(ENoteResult::Bad);
		return ENoteResult::Bad;
	}

	//롱노트 시작점일 경우
	if (BestNote->IsLongNote() && !BestNote->IsLongNoteEnd())
	{
		//TODO : 롱노트 세부판정
		IsSensingLongNote = true;
		Debug::Print(TEXT("Long Note Sense Start"));
		return ENoteResult::None;
	}

	//숏노트일 경우
	ENoteResult Result = ReturnNoteResult(BestNote, NoteToHitComps);
	FString EnumName = StaticEnum<ENoteResult>()->GetNameStringByValue(static_cast<int64>(Result));
	Debug::Print(EnumName);
	BestNote->SpawnRhythmResultWidget(Result);
	OnNoteDetected.Broadcast(Result);
	GetCachedSubsystem()->Release(BestNote);
	
	return Result;
}

ENoteResult ARhythmActor::DetectLongNoteEnd()
{
	if (!IsSensingLongNote) return ENoteResult::None;
	Debug::Print(TEXT("Long Note Sense End"));
	IsSensingLongNote = false;
	TMap<ARhythmNote*, TSet<UPrimitiveComponent*>> NoteToHitComps;
	ARhythmNote* BestNote = GetBestNoteFromLineTrace(NoteToHitComps);
	// 롱노트 감지를 시작했지만 허공에다 마우스를 뗀 경우
	if (!BestNote) return ENoteResult::Bad;
	//롱노트 끝지점을 판정한 경우
	if (BestNote->IsLongNote() && BestNote->IsLongNoteEnd())
	{
		return ReturnNoteResult(BestNote, NoteToHitComps);
	}
	//롱노트 끝을 판정해야 하는데 롱노트 시작점, 혹은 롱노트 중간점, 혹은 숏노트때 마우스를 뗀 경우
	return ENoteResult::Bad;
}

void ARhythmActor::OnInstrumentPickedHandler(EInstrumentType InType)
{
	checkf(InType != EInstrumentType::Invalid, TEXT("InType Is Invalid Type"));
	checkf(NoteHearingComponent, TEXT("NoteHearingComponent is Not valid"));
	IsSensingLongNote = false;

	FocusedType = InType;
	if (InType == EInstrumentType::Background)
	{
		if (NoneSwitch)
		{
			NoteHearingComponent->SetSwitch(NoneSwitch, FString(TEXT("")), FString(TEXT("")));
		}
	}
	else
	{

		if (ARhythmNoteSpawner* FoundSpawner = RhythmNoteSpawners.FindChecked(InType))
		{
			NoteHearingComponent->SetSwitch(FoundSpawner->GetChangeSwitch(), FString(TEXT("")), FString(TEXT("")));
		}
	}
}

void ARhythmActor::CreateAndInitRhythmSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent,
                                              UAkSwitchValue* InChangeSwitch, UAkAudioEvent* InFailEvent)
{
	checkf(!(InType == EInstrumentType::Background || InType == EInstrumentType::Invalid),
		TEXT("InType must NOT be Background or Invalid"));
	if (ARhythmNoteSpawner* NewSpawner = GetOrCreateSpawner(InType))
	{
		NewSpawner->InitSpawner(InType, InNoteEvent, InChangeSwitch, InFailEvent);
	}
}


void ARhythmActor::InitBGMEvent(UAkAudioEvent* InSoundEvent, UAkSwitchValue* InNoneSwitch)
{
	checkf(InSoundEvent, TEXT("SoundEvent is nullptr"));
	checkf(InNoneSwitch, TEXT("NoneSwitch is nullptr"));
	PlayBGMEvent = InSoundEvent;
	NoneSwitch = InNoneSwitch;
}



void ARhythmActor::StartRhythmGame()
{
	checkf(NoneSwitch, TEXT("NoneSwitch is null"));
	checkf(PlayBGMEvent, TEXT("PlayBGMEvent is null"));

	if (NoteSpawnComponent)
	{
		for (TPair<EInstrumentType, TObjectPtr<ARhythmNoteSpawner>>& Elem : RhythmNoteSpawners)
		{
			UAkAudioEvent* SpawnNoteEvent = Elem.Value->GetSpawnNoteEvent();
			FOnAkPostEventCallback Callback;
			Callback.BindUFunction(Elem.Value, FName("OnAkCallback"));
			const int32 CallbackMask = AkCallbackType::AK_MusicSyncUserCue | AkCallbackType::AK_MIDIEvent;
			NoteSpawnComponent->PostAkEvent(
				SpawnNoteEvent,
				CallbackMask,
				Callback
			);
		}
	}
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		this,
		&ThisClass::PlayMusic,
		4.5f,
		false
	);


}


void ARhythmActor::SpawnRhythmRootUI()
{
	if (RhythmUIRootWidgetClass)
	{
		CachedRhythmUIRootWidget = CreateWidget<URhythmUIRootWidget>(GetWorld(), RhythmUIRootWidgetClass);
		if (CachedRhythmUIRootWidget)
		{
			CachedRhythmUIRootWidget->AddToViewport();
		}
	}
}

void ARhythmActor::BeginPlay()
{
	Super::BeginPlay();
	RhythmNoteDestroyer->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnRhythmDestroyBeginOverlap);
	OnInstrumentPicked.AddDynamic(this, &ThisClass::OnInstrumentPickedHandler);
	PrepareRhythmGame();
	NoteSpawnComponent->SetOutputBusVolume(0.f);
	StartRhythmGame();
	EnableInput(GetWorld()->GetFirstPlayerController());
}

void ARhythmActor::PrepareRhythmGame()
{
	SpawnRhythmRootUI();
	if (UTromboneGameInstance* GameInstance = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		FGameplayTag SelectedTag = GameInstance->GetSelectedSongTag();
		if (UGameDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
		{
			FRhythmSongDataRow const* SongRow = DataSubsystem->GetSongRow(SelectedTag);
			if (SongRow)
			{
				InitBGMEvent(SongRow->BgmEvent.Get(), SongRow->NoneSwitch.Get());

				//악기별로 스포너 생성 및 초기화
				for (const FRhythmInstrumentSound& Sound : SongRow->InstrumentSounds)
				{
					EInstrumentType InstrumentType = Sound.InstrumentType;
					UAkAudioEvent* NoteEvent = Sound.NoteEvent.Get();
					UAkSwitchValue* ChangeSwitch = Sound.ChangeSwitch.Get();
					UAkAudioEvent* FailEvent = Sound.FailEvent.Get();
					CreateAndInitRhythmSpawner(InstrumentType, NoteEvent, ChangeSwitch, FailEvent);
				}
			}
		}
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


ENoteResult ARhythmActor::ReturnNoteResult(const ARhythmNote* InNote, const TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InNoteToHitComps) const
{

	if (InNote == nullptr)
	{
		return ENoteResult::Bad;
	}
	const int32 HitCount = InNoteToHitComps.FindChecked(InNote).Num();

	if (HitCount == 1)
	{
		return ENoteResult::Good;
	}
	else if (HitCount == 2)
	{
		return ENoteResult::Great;
	}
	else if (HitCount >= 3)
	{
		return ENoteResult::Excellent; // 또는 Perfect
	}
	else
	{
		checkf(nullptr, TEXT("InNoteToHitComps returned 0 components"));
		return ENoteResult::Invalid;
	}
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
			// 현재 선택된 악기가 아니므로 무시
			if (Note->GetNoteType() != FocusedType)
			{
				continue;
			}
			const double T = Note->GetNoteLifetime();
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
		if (ARhythmNote* Note = Cast<ARhythmNote>(OtherActor))
		{
			if (FocusedType == Note->GetNoteType())
			{
				Note->SpawnRhythmResultWidget(ENoteResult::Bad);
				OnNoteDetected.Broadcast(ENoteResult::Bad);
			}
		}
		GetCachedSubsystem()->Release(OtherActor);
	}
}

void ARhythmActor::PlayMusic()
{
	if (PlayBGMEvent && NoteHearingComponent)
	{
		FOnAkPostEventCallback DummyCallback;
		NoteHearingComponent->PostAkEvent(
			PlayBGMEvent,
			0,
			DummyCallback
		);
	}
}


