// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmActor.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
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
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
#include "Utilities/Defines.h"
#include "Utilities/DebugHelper.h"
#include "Engine/Engine.h"

#if !UE_BUILD_SHIPPING
// 리듬 싱크 디버그. 음악 클럭 화면 표시 + 이후 싱크 로그가 이 값을 본다
TAutoConsoleVariable<int32> CVarRhythmSyncLog(
	TEXT("Trombone.Rhythm.SyncLog"), 0,
	TEXT("1이면 음악 클럭을 화면에 표시하고 BGM 트리거/노트 도달 드리프트를 로그로 찍는다."));
#endif

namespace
{
	// 곡 데이터 로드 재시도 간격과 최대 횟수 (0.5초 × 20 = 10초)
	constexpr float PrepareRetryInterval = 0.5f;
	constexpr int32 MaxPrepareRetryCount = 20;

	// BGM 포스트 재시도. 늦게 시작할수록 노트와 어긋나므로 간격을 짧게 잡는다
	constexpr float BGMPostRetryInterval = 0.2f;
	constexpr int32 MaxBGMPostRetryCount = 5;
}


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

	// 노트 트랙이 실제로 BGMTriggerTimeSec만큼 재생됐을 때 BGM을 시작한다.
	// 트랙 시작이 늦어져도 클럭이 함께 늦으므로 두 트랙 간격은 항상 일정하다
	if (bWaitingToStartBGM)
	{
		if (ARhythmNoteSpawner* Master = GetMasterClockSpawner())
		{
			const double ClockSec = Master->GetMusicTimeSeconds();
			if (ClockSec >= BGMTriggerTimeSec)
			{
#if !UE_BUILD_SHIPPING
				if (CVarRhythmSyncLog.GetValueOnGameThread() != 0)
				{
					UE_LOG(LogTemp, Log, TEXT("[RhythmSync] 클럭 트리거 발화: 클럭 %.1fms / 목표 %.1fms (오버슛 %.1fms)"),
						ClockSec * 1000.0, BGMTriggerTimeSec * 1000.0, (ClockSec - BGMTriggerTimeSec) * 1000.0);
				}
#endif
				PlayMusic();
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (CVarRhythmSyncLog.GetValueOnGameThread() != 0 && GEngine)
	{
		for (const TPair<EInstrumentType, TObjectPtr<ARhythmNoteSpawner>>& Elem : RhythmNoteSpawners)
		{
			ARhythmNoteSpawner* Spawner = Elem.Value;
			if (!IsValid(Spawner))
			{
				continue;
			}
			// 조회가 곧 갱신이다. 노트가 없는 구간에서도 이 호출이 클럭을 굴린다
			const double ClockSec = Spawner->GetMusicTimeSeconds();
			GEngine->AddOnScreenDebugMessage(
				static_cast<uint64>(reinterpret_cast<uintptr_t>(Spawner)),
				0.f,
				Spawner->HasValidMusicClock() ? FColor::Cyan : FColor::Orange,
				FString::Printf(TEXT("[RhythmClock] %s : %.3f s  valid=%d  playingID=%d"),
					*UEnum::GetValueAsString(Elem.Key),
					ClockSec,
					Spawner->HasValidMusicClock() ? 1 : 0,
					Spawner->GetNoteSpawnPlayingID()));
		}
	}
#endif
}

void ARhythmActor::DetectNotes()
{
	if (FocusedType == EInstrumentType::Background || FocusedType == EInstrumentType::Invalid || bCanDetectNotes == false)
	{
		return;
	}
	if (!(GetCachedRhythmSubsystem()->GetCurrentRhythmState() == ERhythmGameState::Playing || GetCachedRhythmSubsystem()->GetCurrentRhythmState() == ERhythmGameState::Start))
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
		bIsSensingLongNote = true;
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
	if (!bIsSensingLongNote) return ENoteResult::None;
	Debug::Print(TEXT("Long Note Sense End"));
	bIsSensingLongNote = false;
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

void ARhythmActor::PrepareAndStartRhythmGame(const FGameplayTag& InSelectedTag)
{
	// 데이터 로딩이 완료된 경우 즉시 시작 대기열 진입
	if (bIsDataLoaded && InSelectedTag == LoadedGameplayTag)
	{
		//TODO : 싱글플레이어에서도 가능하게 하기
		//bAreOtherPlayersReady = true;
		WaitForOtherPlayers();
		
		return;
	}

	// 중복되서 PrepareAndStartRhythmGame 호출한 경우.
	// 현재 로딩 중이라면, 완료되는 즉시 시작되도록 예약 플래그 설정
	if (bIsLoadingData)
	{
		bStartRequested = true;
		return;
	}

	//로딩이 안된 경우 로딩 트리거
	CleanupRhythmGame();
	bStartRequested = true;
	PrepareRhythmGame(InSelectedTag);
}




void ARhythmActor::PauseRhythmGame()
{
	// 노트 트랙은 Wwise가 멈추는데 이 타이머는 계속 흘러 BGM이 먼저 시작되던 문제
	GetWorldTimerManager().PauseTimer(PlayBackgroundMusicTimerHandle);

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (BGMPlayingID != 0 && BGMPlayingID != AK_INVALID_PLAYING_ID)
	{

		if (auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Pause, BGMPlayingID);
		}
	}
	for (TPair<EInstrumentType, TObjectPtr<ARhythmNoteSpawner>>& Elem : RhythmNoteSpawners)
	{
		ARhythmNoteSpawner* Spawner = Elem.Value;
		if (!IsValid(Spawner))
		{
			continue;
		}

		if (AudioDevice && Spawner->GetNoteSpawnPlayingID() && Spawner->GetNoteSpawnPlayingID() != AK_INVALID_PLAYING_ID)
		{
			Spawner->PauseRhythmGame();
		}
	}
}

void ARhythmActor::ResumeRhythmGame()
{
	GetWorldTimerManager().UnPauseTimer(PlayBackgroundMusicTimerHandle);

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (BGMPlayingID != 0 && BGMPlayingID != AK_INVALID_PLAYING_ID)
	{
		if (auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Resume, BGMPlayingID);
		}

	}
	for (TPair<EInstrumentType, TObjectPtr<ARhythmNoteSpawner>>& Elem : RhythmNoteSpawners)
	{
		ARhythmNoteSpawner* Spawner = Elem.Value;
		if (!IsValid(Spawner))
		{
			continue;
		}

		if (AudioDevice && Spawner->GetNoteSpawnPlayingID() && Spawner->GetNoteSpawnPlayingID() != AK_INVALID_PLAYING_ID)
		{
			Spawner->ResumeRhythmGame();
		}
	}
}

void ARhythmActor::StopRhythmGame()
{
	for (auto& Elem : RhythmNoteSpawners)
	{
		if (ARhythmNoteSpawner* Spawner = Elem.Value.Get())
		{
			Spawner->StopRhythmGame();
		}
	}
	CleanupRhythmGame();
}

void ARhythmActor::BeginPlay()
{
	Super::BeginPlay();
	InitGameState();
	RhythmNoteDestroyer->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnRhythmDestroyBeginOverlap);
	GetCachedActorPoolSubsystem();
	GetCachedRhythmSubsystem()->OnInstrumentPicked.AddDynamic(this, &ThisClass::OnInstrumentPickedHandler);
	GetCachedRhythmSubsystem()->OnNoteDetected.AddDynamic(this, &ThisClass::OnNoteDetectedHandler);
	GetCachedRhythmSubsystem()->RegisterRhythmActor(this);
	NoteSpawnComponent->SetOutputBusVolume(0.f);
	if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		PrepareRhythmGame(GI->GetSelectedSongTag());
	}
}

void ARhythmActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupRhythmGame();
	Super::EndPlay(EndPlayReason);
}

void ARhythmActor::CleanupRhythmGame()
{
	// 모든 타이머 정지
	GetWorldTimerManager().ClearTimer(GameStateInitTimerHandle);
	GameStateInitTimerHandle.Invalidate();

	GetWorldTimerManager().ClearTimer(CheckPlayersTimerHandle);
	CheckPlayersTimerHandle.Invalidate();

	GetWorldTimerManager().ClearTimer(PlayBackgroundMusicTimerHandle);
	PlayBackgroundMusicTimerHandle.Invalidate();

	GetWorldTimerManager().ClearTimer(PrepareRetryTimerHandle);
	PrepareRetryTimerHandle.Invalidate();

	// 사운드 엔진 정지
	if (BGMPlayingID != 0 && BGMPlayingID != AK_INVALID_PLAYING_ID)
	{
		if (auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			SoundEngine->StopPlayingID(BGMPlayingID);
		}
		BGMPlayingID = 0;
	}

	
	for (auto& Elem : RhythmNoteSpawners)
	{
		if (IsValid(Elem.Value))
		{
			Elem.Value->Destroy();
		}
	}
	RhythmNoteSpawners.Empty();

	// 상태 및 플래그 초기화
	bIsDataLoaded = false;
	LoadedGameplayTag = FGameplayTag::EmptyTag;
	bIsLoadingData = false;
	bStartRequested = false;
	PrepareRetryCount = 0;
	BGMPostRetryCount = 0;
	bIsSensingLongNote = false;
	bHasReceivedMusicStartCallback = false;
	bHasReceivedDurationCallback = false;
	bHasShotBGMDelegate = false;
	bWaitingToStartBGM = false;

	if (CachedRhythmUIRootWidget)
	{
		CachedRhythmUIRootWidget->RemoveFromParent();
		CachedRhythmUIRootWidget = nullptr;
	}
}

void ARhythmActor::PrepareRhythmGame(const FGameplayTag& InGamePlayTag)
{
	if (bIsLoadingData) return;

	bIsLoadingData = true;
	SpawnRhythmRootUI();

	bool bDataLoadedSuccessfully = false;
	BGMPlayingID = 0;

	if (!InGamePlayTag.IsValid())
	{
		Debug::Print(TEXT("[RhythmActor] Client SelectedTag is Invalid! Data might not be synced yet."), -1, FColor::Red);
	}

	if (UGameDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
	{
		FRhythmSongDataRow const* SongRow = DataSubsystem->GetSongRow(InGamePlayTag);
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
				LoadedGameplayTag = InGamePlayTag;
				bDataLoadedSuccessfully = true;
			}
			else
			{
				Debug::Print(TEXT("[RhythmActor] SongRow found but BgmEvent is NULL!"), -1, FColor::Red);
			}
		}
		else
		{
			Debug::Print(FString::Printf(TEXT("[RhythmActor] SongRow Not Found for Tag: %s"), *InGamePlayTag.ToString()), -1, FColor::Yellow);
		}
	}
	bIsLoadingData = false;

	if (bDataLoadedSuccessfully)
	{
		bIsDataLoaded = true;
		PrepareRetryCount = 0;
		GetWorldTimerManager().ClearTimer(PrepareRetryTimerHandle);

		// 로딩 완료 시점에 예약된 시작 요청이 있었다면 실행
		if (bStartRequested)
		{
			bStartRequested = false;
			WaitForOtherPlayers();
		}
		else
		{
			Debug::Print(TEXT("[RhythmActor] PrepareRhythmGame completed. Waiting for Start Request..."), -1, FColor::Green);
		}
	}
	else
	{
		Debug::Print(TEXT("[RhythmActor] PrepareRhythmGame Failed to load data. Music will not play."), -1, FColor::Red);

		// 여기서 멈추면 이 머신은 종료 보고도 못 해서 전원이 결과 레벨로 못 간다
		if (PrepareRetryCount < MaxPrepareRetryCount)
		{
			++PrepareRetryCount;
			GetWorldTimerManager().SetTimer(PrepareRetryTimerHandle, this, &ThisClass::RetryPrepareRhythmGame, PrepareRetryInterval, false);
		}
		else
		{
			Debug::Print(FString::Printf(TEXT("[RhythmActor] Prepare retry limit reached (%d). Tag: %s"),
				MaxPrepareRetryCount, *InGamePlayTag.ToString()), -1, FColor::Red);
		}
	}
}

FGameplayTag ARhythmActor::GetSelectedSongTagFromGameInstance() const
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		return GI->GetSelectedSongTag();
	}
	return FGameplayTag::EmptyTag;
}

void ARhythmActor::RetryPrepareRhythmGame()
{
	if (bIsDataLoaded)
	{
		return;
	}

	// 태그 자체가 문제였을 수 있으니 매번 원본을 다시 읽는다
	FGameplayTag RetryTag = GetSelectedSongTagFromGameInstance();
	if (!RetryTag.IsValid())
	{
		RetryTag = LoadedGameplayTag;
	}

	Debug::Print(FString::Printf(TEXT("[RhythmActor] Retrying Prepare %d/%d. Tag: %s"),
		PrepareRetryCount, MaxPrepareRetryCount, *RetryTag.ToString()), -1, FColor::Yellow);

	PrepareRhythmGame(RetryTag);
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

	// EnableGetMusicPlayPosition이 있어야 스포너가 GetPlayingSegmentInfo로 재생 위치를 읽을 수 있다
	const int32 CallbackMask = AkCallbackType::AK_MusicSyncUserCue | AkCallbackType::AK_EnableGetMusicPlayPosition;

	for (const FSpawnEventInfo& Info : SpawnEvents)
	{
		FOnAkPostEventCallback Callback;
		Callback.BindUFunction(Info.Spawner, FName("OnAkCallback"));

		int32 NoteSpawnPlayingID = NoteSpawnComponent->PostAkEvent(
			Info.Event,
			CallbackMask,
			Callback
		);
		if (NoteSpawnPlayingID != 0 && NoteSpawnPlayingID != AK_INVALID_PLAYING_ID)
		{
			Info.Spawner->SetNoteSpawnPlayingID(NoteSpawnPlayingID);
		}
	}

	// 기기별 출력 지연 보정값. 양수면 그만큼 BGM을 일찍 시작한다
	int32 OffsetMs = 0;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const USaveManagerSubsystem* SaveManager = GI->GetSubsystem<USaveManagerSubsystem>())
		{
			OffsetMs = SaveManager->GetAudioSettings().RhythmAudioOffsetMs;
		}
	}

	// 노트 이동 시간과 같은 값을 써야 노트 도달과 BGM의 같은 음이 겹친다
	float NoteTravelTime = 3.f;
	for (const FSpawnEventInfo& Info : SpawnEvents)
	{
		NoteTravelTime = Info.Spawner->TimeToComplete;
		break;
	}

	BGMTriggerTimeSec = NoteTravelTime - OffsetMs / 1000.0;
	bWaitingToStartBGM = true;

	// 안전망: 클럭이 끝내 안 살아나면 기존 방식으로라도 재생한다
	GetWorldTimerManager().SetTimer(
		PlayBackgroundMusicTimerHandle,
		this,
		&ThisClass::PlayMusic,
		5.0f,
		false
	);
}

ARhythmNoteSpawner* ARhythmActor::GetMasterClockSpawner()
{
	for (const TPair<EInstrumentType, TObjectPtr<ARhythmNoteSpawner>>& Elem : RhythmNoteSpawners)
	{
		ARhythmNoteSpawner* Spawner = Elem.Value;
		if (!IsValid(Spawner))
		{
			continue;
		}
		// 조회가 곧 갱신이다. 아직 무효한 스포너도 이 호출로 클럭이 살아난다
		Spawner->GetMusicTimeSeconds();
		if (Spawner->HasValidMusicClock())
		{
			return Spawner;
		}
	}
	return nullptr;
}

void ARhythmActor::CreateAndInitRhythmSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent,
	UAkSwitchValue* InChangeSwitch, UAkAudioEvent* InFailEvent)
{
	checkf(!(InType == EInstrumentType::Background || InType == EInstrumentType::Invalid),
		TEXT("InType must NOT be Background or Invalid"));
	if (ARhythmNoteSpawner* NewSpawner = GetOrCreateSpawner(InType))
	{
		NewSpawner->InitSpawner(InType, InNoteEvent, InChangeSwitch, InFailEvent, bIsSyncTesting);
	}
}


void ARhythmActor::InitBGMEvent(UAkAudioEvent* InSoundEvent, UAkSwitchValue* InNoneSwitch)
{
	checkf(InSoundEvent, TEXT("SoundEvent is nullptr"));
	checkf(InNoneSwitch, TEXT("NoneSwitch is nullptr"));
	PlayBGMEvent = InSoundEvent;
	NoneSwitch = InNoneSwitch;
}

void ARhythmActor::SpawnRhythmRootUI()
{
	// 로드를 재시도할 때마다 새로 만들면 위젯이 화면에 겹쳐 쌓인다
	if (IsValid(CachedRhythmUIRootWidget))
	{
		return;
	}

	if (RhythmUIRootWidgetClass)
	{
		CachedRhythmUIRootWidget = CreateWidget<URhythmUIRootWidget>(GetWorld(), RhythmUIRootWidgetClass);
		if (CachedRhythmUIRootWidget)
		{
			CachedRhythmUIRootWidget->AddToViewport();
		}
	}
}

void ARhythmActor::InitGameState()
{
	//Multiplayer에선 GameState가 늦게 세팅될 수 있으므로, timer를 걸어서 Delegate 바인딩 재시도
	AInGameState* InGameState = GetWorld()->GetGameState<AInGameState>();

	if (InGameState)
	{
		InGameState->OnInGameStateChanged.RemoveDynamic(this, &ThisClass::HandleInGameStateChanged);
		InGameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);
		if (GetNetMode() == NM_Standalone || InGameState->GetCurrentGameState() == EInGameState::Play)
		{
			HandleInGameStateChanged(EInGameState::Play);
		}
		GetWorldTimerManager().ClearTimer(GameStateInitTimerHandle);
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
	bIsSensingLongNote = false;

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

		if (TObjectPtr<ARhythmNoteSpawner>* FoundPtr = RhythmNoteSpawners.Find(NewType))
		{
			ARhythmNoteSpawner* FoundSpawner = FoundPtr->Get();
			if (IsValid(FoundSpawner))
			{
				NoteHearingComponent->SetSwitch(FoundSpawner->GetChangeSwitch(), FString(TEXT("")), FString(TEXT("")));
			}
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
		{
			bAreOtherPlayersReady = true;

			// 로드에 실패했으면 LoadedGameplayTag가 비어 있다. 그걸 그대로 쓰면 재시도도 같이 실패한다
			FGameplayTag StartTag = GetSelectedSongTagFromGameInstance();
			if (!StartTag.IsValid())
			{
				StartTag = LoadedGameplayTag;
			}

			if (StartTag.IsValid())
			{
				GetCachedRhythmSubsystem()->StartRhythmGame(StartTag);
			}
			else
			{
				Debug::Print(TEXT("[RhythmActor] Play received but SelectedTag is Empty! Cannot start."), -1, FColor::Red);
			}
		}
			break;
		
		default: ;
	}
}

void ARhythmActor::WaitForOtherPlayers()
{
	AInGameState* InGameState = GetWorld() ? GetWorld()->GetGameState<AInGameState>() : nullptr;
	if (InGameState)
	{
		if (InGameState->GetCurrentGameState() == EInGameState::Play)
		{
			bAreOtherPlayersReady = true;
		}
	}
	if (GetNetMode() == NM_Standalone)
	{
		bAreOtherPlayersReady = true;
	}

	if (bIsDataLoaded && bAreOtherPlayersReady)
	{
		StartRhythmGame();
		EnableInput(GetWorld()->GetFirstPlayerController());
		GetWorldTimerManager().ClearTimer(CheckPlayersTimerHandle);
	}
	else
	{
		FString Reason = !bIsDataLoaded ? TEXT("DataNotLoaded") : TEXT("OtherPlayersNotReady");
		Debug::Print(FString::Printf(TEXT("[RhythmActor] Waiting... Reason: %s"), *Reason), -1, FColor::Yellow);

		GetWorld()->GetTimerManager().SetTimer(
			CheckPlayersTimerHandle,
			this,
			&ARhythmActor::WaitForOtherPlayers,
			0.5f,
			false
		);
	}
}

void ARhythmActor::PlayMusic()
{
	// 클럭 트리거와 안전망 타이머 중 어느 쪽이 먼저 와도 한 번만 재생한다
	bWaitingToStartBGM = false;
	GetWorldTimerManager().ClearTimer(PlayBackgroundMusicTimerHandle);

	if (PlayBGMEvent && NoteHearingComponent)
	{
		FOnAkPostEventCallback Callback;
		Callback.BindUFunction(this, FName("HandleBGMCallbacks"));
		UGameDataSubsystem* GameDataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>();

		const int32 CallbackMask = AkCallbackType::AK_MusicPlayStarted | AkCallbackType::AK_Duration | AkCallbackType::AK_MusicSyncUserCue | AkCallbackType::AK_MusicSyncEntry|
			AkCallbackType::AK_EndOfEvent | AkCallbackType::AK_EnableGetSourcePlayPosition |AkCallbackType::AK_EnableGetMusicPlayPosition;
		bHasReceivedDurationCallback = false;
		bHasReceivedMusicStartCallback = false;
		bHasShotBGMDelegate = false;

		BGMPlayingID = NoteHearingComponent->PostAkEvent(
			PlayBGMEvent,
			CallbackMask,
			Callback);

		if (BGMPlayingID != 0 && GameDataSubsystem)
		{
			GameDataSubsystem->SetCurrentSongPlayingID(BGMPlayingID);
		}

#if !UE_BUILD_SHIPPING
		// 목표(BGMTriggerTimeSec)와 크게 다르면 클럭 트리거가 아니라 5초 안전망으로 들어온 것이다
		if (CVarRhythmSyncLog.GetValueOnGameThread() != 0)
		{
			double ClockSec = -1.0;
			if (ARhythmNoteSpawner* Master = GetMasterClockSpawner())
			{
				ClockSec = Master->GetMusicTimeSeconds();
			}
			UE_LOG(LogTemp, Log, TEXT("[RhythmSync] BGM Post 시점 클럭 %.1fms (목표 %.1fms)"),
				ClockSec * 1000.0, BGMTriggerTimeSec * 1000.0);
		}
#endif
	}

	// 포스트에 실패하면 EndOfEvent가 안 와서 이 머신만 종료 보고를 못 하고, 전원이 결과 레벨로 못 간다
	const bool bPostSucceeded = (BGMPlayingID != 0 && BGMPlayingID != AK_INVALID_PLAYING_ID);
	if (bPostSucceeded)
	{
		BGMPostRetryCount = 0;
	}
	else if (BGMPostRetryCount < MaxBGMPostRetryCount)
	{
		++BGMPostRetryCount;
		Debug::Print(FString::Printf(TEXT("[RhythmActor] BGM PostAkEvent failed. Retrying %d/%d"),
			BGMPostRetryCount, MaxBGMPostRetryCount), -1, FColor::Red);

		GetWorldTimerManager().SetTimer(PlayBackgroundMusicTimerHandle, this, &ThisClass::PlayMusic, BGMPostRetryInterval, false);
	}
	else
	{
		Debug::Print(FString::Printf(TEXT("[RhythmActor] BGM PostAkEvent failed %d times. Song will never end."),
			MaxBGMPostRetryCount), -1, FColor::Red);
	}
}

void ARhythmActor::HandleBGMCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	GetCachedRhythmSubsystem()->HandleMusicCallbacksFromRhythmActor(CallbackType, CallbackInfo);
	if (UGameDataSubsystem* GameDataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
	{
		GameDataSubsystem->HandleMusicCallbacks(CallbackType, CallbackInfo);
	}
	switch (CallbackType)
	{
	case EAkCallbackType::Duration:
	{
		bHasReceivedDurationCallback = true;
	}
	break;
	case EAkCallbackType::MusicPlayStarted:
	{
		bHasReceivedMusicStartCallback = true;
#if !UE_BUILD_SHIPPING
		// BGM이 실제로 소리를 내기 시작한 순간의 노트 트랙 위치 = 두 트랙의 실제 간격
		if (CVarRhythmSyncLog.GetValueOnGameThread() != 0)
		{
			if (ARhythmNoteSpawner* Master = GetMasterClockSpawner())
			{
				UE_LOG(LogTemp, Log, TEXT("[RhythmSync] 두 트랙 실제 간격 %.1fms (목표 %.1fms)"),
					Master->GetMusicTimeSeconds() * 1000.0, BGMTriggerTimeSec * 1000.0);
			}
		}
#endif
	}
	break;
	case EAkCallbackType::MusicSyncUserCue:
	{
		if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
		{
			const FString& CueString = MusicInfo->UserCueName;
			if (!CueString.IsEmpty())
			{
				const FName CueName(*CueString);
				if (CueName == TEXT("Event_Enable_Click"))
				{
					bCanDetectNotes = true;
				}
				if (CueName == TEXT("Event_Disable_Click"))
				{
					bCanDetectNotes = false;
				}
			}
		}
	}
	break;
	}
	//MusicPlayStart Callback이 받은 시점에서 리듬게임 시작했다고 알림.
	if (!bHasShotBGMDelegate)
	{
		if (bHasReceivedDurationCallback && bHasReceivedMusicStartCallback)
		{
			bHasShotBGMDelegate = true;
			GetCachedRhythmSubsystem()->OnRhythmGameStateChanged.Broadcast(ERhythmGameState::Start);
		}
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

void ARhythmActor::OnRhythmDestroyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsSyncTesting) return;
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