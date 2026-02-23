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
#include "Framework/InGameState.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"
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

void ARhythmActor::DetectNotes()
{
	if (FocusedType == EInstrumentType::Background || FocusedType == EInstrumentType::Invalid)
	{
		return;
	}
	TMap<ARhythmNote*, TSet<UPrimitiveComponent*>> NoteToHitComps;
	ARhythmNote* BestNote = GetBestNoteFromLineTrace(NoteToHitComps);
	if (!BestNote)
	{
		GetCachedRhythmSubsystem()->OnNoteDetected.Broadcast(ENoteResult::Bad);
		return;
	}

	//롱노트 시작점일 경우
	if (BestNote->IsLongNote() && !BestNote->IsLongNoteEnd())
	{
		//TODO : 롱노트 세부판정
		IsSensingLongNote = true;
		Debug::Print(TEXT("Long Note Sense Start"));
		return;
	}

	//숏노트일 경우
	ENoteResult Result = ReturnNoteResult(BestNote, NoteToHitComps);
	FString EnumName = StaticEnum<ENoteResult>()->GetNameStringByValue(static_cast<int64>(Result));
	BestNote->SpawnRhythmResultWidget(Result);
	if (Result == ENoteResult::Good || Result == ENoteResult::Excellent)
	{
		BestNote->CancelSyncDebugTimer();
	}
	GetCachedRhythmSubsystem()->OnNoteDetected.Broadcast(Result);
	GetCachedActorPoolSubsystem()->Release(BestNote);
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

void ARhythmActor::CreateAndInitRhythmSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent,
                                              UAkSwitchValue* InChangeSwitch, UAkAudioEvent* InFailEvent)
{
	checkf(!(InType == EInstrumentType::Background || InType == EInstrumentType::Invalid),
		TEXT("InType must NOT be Background or Invalid"));
	if (ARhythmNoteSpawner* NewSpawner = GetOrCreateSpawner(InType))
	{
		NewSpawner->InitSpawner(InType, InNoteEvent, InChangeSwitch, InFailEvent, IsSyncTesting);
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

	if (!NoteSpawnComponent) return;
	struct FSpawnEventInfo
	{
		UAkAudioEvent* Event = nullptr;
		ARhythmNoteSpawner* Spawner = nullptr;
	};

	TArray<FSpawnEventInfo> SpawnEvents;
	SpawnEvents.Reserve(RhythmNoteSpawners.Num());

	for (TPair<EInstrumentType, TObjectPtr<ARhythmNoteSpawner>>& Elem : RhythmNoteSpawners)
	{
		ARhythmNoteSpawner* Spawner = Elem.Value;
		if (!IsValid(Spawner))
		{
			continue;
		}

		UAkAudioEvent* SpawnNoteEvent = Spawner->GetSpawnNoteEvent();
		if (!SpawnNoteEvent)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("ARhythmActor::StartRhythmGame - SpawnNoteEvent is null. Spawner : %s"),
				*GetNameSafe(Spawner));
			continue;
		}

		SpawnEvents.Add({ SpawnNoteEvent, Spawner });
	}

	const int32 CallbackMask = AkCallbackType::AK_MusicSyncUserCue; // | AkCallbackType::AK_MIDIEvent;

	for (const FSpawnEventInfo& Info : SpawnEvents)
	{
		FOnAkPostEventCallback Callback;
		Callback.BindUFunction(Info.Spawner, FName("OnAkCallback"));

		NoteSpawnComponent->PostAkEvent(
			Info.Event,
			CallbackMask,
			Callback
		);
	}

	GetWorldTimerManager().SetTimer(
		PlayBackgroundMusicTimerHandle,
		this,
		&ThisClass::PlayMusic,
		2.9f,
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
	InitGameState();
	RhythmNoteDestroyer->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnRhythmDestroyBeginOverlap);
	GetCachedActorPoolSubsystem();
	GetCachedRhythmSubsystem()->OnInstrumentPicked.AddDynamic(this, &ThisClass::OnInstrumentPickedHandler);
	GetCachedRhythmSubsystem()->OnNoteDetected.AddDynamic(this, &ThisClass::OnNoteDetectedHandler);
	NoteSpawnComponent->SetOutputBusVolume(0.f);
	PrepareRhythmGame();
}

void ARhythmActor::InitGameState()
{
	//Multiplayer에선 GameState가 늦게 세팅될 수 있으므로, timer를 걸어서 Delegate 바인딩 재시도
	AInGameState* InGameState = GetWorld()->GetGameState<AInGameState>();

	if (InGameState)
	{
		InGameState->OnInGameStateChanged.RemoveDynamic(this, &ThisClass::HandleInGameStateChanged);
		InGameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);
		if (InGameState->GetCurrentGameState() == EInGameState::Play)
		{
			HandleInGameStateChanged(EInGameState::Play);
		}
		GetWorldTimerManager().ClearTimer(GameStateInitTimerHandle);
		Debug::Print(TEXT("[RhythmActor] GameState Initialized Successfully."), -1, FColor::Green);
	}
	else
	{
		Debug::Print(TEXT("[RhythmActor] GameState is NULL. Retrying in 0.1s..."), -1, FColor::Yellow);
		GetWorldTimerManager().SetTimer(
			GameStateInitTimerHandle,
			this,
			&ThisClass::InitGameState,
			0.1f,
			false
		);
	}
}

void ARhythmActor::PrepareRhythmGame()
{
	SpawnRhythmRootUI();
	bool bDataLoadedSuccessfully = false;
	if (UTromboneGameInstance* GameInstance = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		FGameplayTag SelectedTag = GameInstance->GetSelectedSongTag();
		if (!SelectedTag.IsValid())
		{
			Debug::Print(TEXT("[RhythmActor] Client SelectedTag is Invalid! Data might not be synced yet."), -1, FColor::Red);
		}
		if (UGameDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
		{
			FRhythmSongDataRow const* SongRow = DataSubsystem->GetSongRow(SelectedTag);
			if (SongRow)
			{
				UAkAudioEvent* SongBgmEvent = SongRow->BgmEvent.LoadSynchronous();
				UAkSwitchValue* SongNoneSwitch = SongRow->NoneSwitch.LoadSynchronous();
				if (SongBgmEvent)
                {
					InitBGMEvent(SongBgmEvent, SongNoneSwitch);

					//악기별로 스포너 생성 및 초기화
					for (const FRhythmInstrumentSound& Sound : SongRow->InstrumentSounds)
					{
						EInstrumentType InstrumentType = Sound.InstrumentType;
						UAkAudioEvent* NoteEvent = Sound.NoteEvent.LoadSynchronous();
						UAkSwitchValue* ChangeSwitch = Sound.ChangeSwitch.LoadSynchronous();
						UAkAudioEvent* FailEvent = Sound.FailEvent.LoadSynchronous();
						CreateAndInitRhythmSpawner(InstrumentType, NoteEvent, ChangeSwitch, FailEvent);
					}
                    bDataLoadedSuccessfully = true;
                }
                else
                {
					Debug::Print(TEXT("[RhythmActor] SongRow found but BgmEvent is NULL!"), -1, FColor::Red);
                }
			}
			else
			{
				Debug::Print(FString::Printf(TEXT("[RhythmActor] SongRow Not Found for Tag: %s"), *SelectedTag.ToString()), -1, FColor::Yellow);
			}
		}
	}
	if (bDataLoadedSuccessfully)
	{
		IsRhythmGameReady = true;
		WaitForOtherPlayers();
	}
	else
	{
		Debug::Print(TEXT("[RhythmActor] PrepareRhythmGame Failed to load data. Music will not play."), -1, FColor::Red);
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

void ARhythmActor::OnInstrumentPickedHandler(EInstrumentType PrevType, EInstrumentType NewType)
{
	checkf(NewType != EInstrumentType::Invalid, TEXT("InType Is Invalid Type"));
	checkf(NoteHearingComponent, TEXT("NoteHearingComponent is Not valid"));
	IsSensingLongNote = false;

	FocusedType = NewType;
	if (NewType == EInstrumentType::Background)
	{
		if (NoneSwitch)
		{
			NoteHearingComponent->SetSwitch(NoneSwitch, FString(TEXT("")), FString(TEXT("")));
		}
		if (RhythmNoteDestroyer)
		{
			RhythmNoteDestroyer->SetBoxExtent(FVector(100.f, 100.f, 100.f));
		}
	}
	else
	{

		if (ARhythmNoteSpawner* FoundSpawner = RhythmNoteSpawners.FindChecked(NewType))
		{
			NoteHearingComponent->SetSwitch(FoundSpawner->GetChangeSwitch(), FString(TEXT("")), FString(TEXT("")));
		}
		if (RhythmNoteDestroyer)
		{
			RhythmNoteDestroyer->SetBoxExtent(FVector(RhythmDestroyerBoxExtent, RhythmDestroyerBoxExtent, RhythmDestroyerBoxExtent));
		}
	}
}

void ARhythmActor::OnNoteDetectedHandler(ENoteResult InNoteResult)
{
	if (InNoteResult == ENoteResult::Bad)
	{
		if (ARhythmNoteSpawner* FoundSpawner = RhythmNoteSpawners.FindChecked(FocusedType))
		{
			if (NoteHearingComponent && FoundSpawner->GetFailEvent())
			{
				NoteHearingComponent->PostAkEvent(
					FoundSpawner->GetFailEvent(),
					0,
					FOnAkPostEventCallback()
				);
			}
		}

	}
}

void ARhythmActor::HandleInGameStateChanged(EInGameState InGameState)
{
	switch (InGameState) {
		case EInGameState::Play:
			AreOtherPlayersReady = true;
			break;
		
		case EInGameState::End:
			CachedRhythmUIRootWidget->OnGameEnded();
			break;
		
		default: ;
	}
}

void ARhythmActor::WaitForOtherPlayers()
{
	if (IsRhythmGameReady && AreOtherPlayersReady)
	{
		StartRhythmGame();
		EnableInput(GetWorld()->GetFirstPlayerController());
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			CheckPlayersTimerHandle,
			this,
			&ARhythmActor::WaitForOtherPlayers,
			1.0f,
			false
		);
	}
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
	else if (HitCount >= 2)
	{
		return ENoteResult::Excellent;
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

UActorPoolSubsystem* ARhythmActor::GetCachedActorPoolSubsystem()
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

URhythmSubsystem* ARhythmActor::GetCachedRhythmSubsystem()
{
	if (CachedRhythmSubsystem.IsValid())
		return CachedRhythmSubsystem.Get();

	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		CachedRhythmSubsystem = RhythmSubsystem;
		return CachedRhythmSubsystem.Get();
	}

	return nullptr;
}

void ARhythmActor::OnRhythmDestroyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsSyncTesting) return;
	if (OtherActor && OtherActor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
	{
		if (ARhythmNote* Note = Cast<ARhythmNote>(OtherActor))
		{
			Note->CancelSyncDebugTimer();
			if (FocusedType == Note->GetNoteType())
			{
				Note->SpawnRhythmResultWidget(ENoteResult::Bad);
				GetCachedRhythmSubsystem()->OnNoteDetected.Broadcast(ENoteResult::Bad);
			}
		}
		GetCachedActorPoolSubsystem()->Release(OtherActor);
	}
}

void ARhythmActor::PlayMusic()
{
	if (PlayBGMEvent && NoteHearingComponent)
	{
		FOnAkPostEventCallback Callback;
		Callback.BindUFunction(this, FName("HandleBGMCallbacks"));
		UGameDataSubsystem* GameDataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>();

		const int32 CallbackMask = AkCallbackType::AK_MusicPlayStarted | AkCallbackType::AK_Duration | AkCallbackType::AK_MusicSyncUserCue | AkCallbackType::AK_EndOfEvent;
		hasReceivedDurationCallback = false;
		hasReceivedMusicStartCallback = false;
		hasShotBGMDelegate = false;
		int32 PlayingID = NoteHearingComponent->PostAkEvent(
			PlayBGMEvent,
			CallbackMask,
			Callback);
		if (PlayingID != 0 && GameDataSubsystem)
		{
			GameDataSubsystem->SetCurrentSongPlayingID(PlayingID);
		}

	}
}

void ARhythmActor::HandleBGMCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	GetCachedRhythmSubsystem()->HandleMusicCallbacks(CallbackType, CallbackInfo);
	if (UGameDataSubsystem* GameDataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
	{
		GameDataSubsystem->HandleMusicCallbacks(CallbackType, CallbackInfo);
	}
	switch (CallbackType)
	{
	case EAkCallbackType::Duration:
		{
			hasReceivedDurationCallback = true;
		}
		break;
	case EAkCallbackType::MusicPlayStarted:
		{
			hasReceivedMusicStartCallback = true;
		}
		break;
	}
	if (!hasShotBGMDelegate)
	{
		if (hasReceivedDurationCallback && hasReceivedMusicStartCallback)
		{
			hasShotBGMDelegate = true;
			GetCachedRhythmSubsystem()->OnRhythmGameStarted.Broadcast();
		}
	}
	
}
