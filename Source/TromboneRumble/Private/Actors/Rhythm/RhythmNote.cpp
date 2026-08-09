// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNote.h"

#include "Actors/Rhythm/RhythmActor.h"
#include "Components/SphereComponent.h"
#include "Components/ActorComponents/RhythmNoteUIControllerComponent.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetBase.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"
#include "Actors/Rhythm/NoteVisualizer.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/DebugHelper.h"

#if !UE_BUILD_SHIPPING
extern TAutoConsoleVariable<int32> CVarRhythmSyncLog; // 정의: RhythmActor.cpp
#endif

ARhythmNote::ARhythmNote()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	OuterSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OuterSphere"));
	OuterSphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	OuterSphere->SetupAttachment(RootComponent);
	InnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InnerSphere"));
	InnerSphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	InnerSphere->SetupAttachment(RootComponent);

	RhythmNoteUIControllerComponent = CreateDefaultSubobject<URhythmNoteUIControllerComponent>(TEXT("RhythmNoteUIControllerComponent"));
}

void ARhythmNote::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsMoving) return;

	// 클럭이 죽었을 때의 폴백 겸 기존 소비자를 위해 계속 누적한다
	NoteLifeTime += DeltaTime;

	float MoveAlpha = NoteLifeTime / TimeToComplete;
	if (ARhythmNoteSpawner* Spawner = ParentSpawner.Get())
	{
		const double MusicTime = Spawner->GetMusicTimeSeconds();
		if (Spawner->HasValidMusicClock())
		{
			MoveAlpha = static_cast<float>((MusicTime - SpawnMusicTimeSec) / TimeToComplete);
		}
	}

	// 판정선(Alpha 1.0)에서 멈추지 않고 같은 속도로 지나간다. 소멸은 Destroyer가 맡는다
	SetActorLocation(FMath::Lerp(StartLocation, EndLocation, FMath::Max(MoveAlpha, 0.f)));

	// 판정선 도달 순간의 오차. 클럭이 정상이면 0에 가까워야 한다
	if (!bSyncArrivalLogged && MoveAlpha >= 1.f)
	{
		bSyncArrivalLogged = true;
#if !UE_BUILD_SHIPPING
		if (CVarRhythmSyncLog.GetValueOnGameThread() != 0 && ParentSpawner.IsValid() && ParentSpawner->HasValidMusicClock())
		{
			const double DriftMs = (ParentSpawner->GetMusicTimeSeconds() - SpawnMusicTimeSec - TimeToComplete) * 1000.0;
			UE_LOG(LogTemp, Log, TEXT("[RhythmSync] 노트 도달 드리프트 %.1f ms"), DriftMs);
		}
#endif
	}

	// Destroyer가 못 잡았을 때 풀이 새지 않도록 회수한다
	if (MoveAlpha > NoteBackstopAlpha)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("RhythmNote가 Destroyer에 잡히지 않아 백스톱 회수됨 - BP_RhythmActor의 Note Destroyer 배치를 확인할 것"));
		if (CachedActorPoolSubsystem.IsValid())
		{
			CachedActorPoolSubsystem->Release(this);
		}
		return;
	}

	const float ProgressAlpha = FMath::Clamp(MoveAlpha, 0.f, 1.f);
	if (CachedRhythmNoteChannelSubsystem.IsValid())
	{
		CachedRhythmNoteChannelSubsystem->UpdateProgress(NoteHandle.Id, ProgressAlpha);
	}
}

void ARhythmNote::SetPause(bool InPause)
{
	bIsMoving = !InPause;
	if (SyncDebugTimerHandle.IsValid())
	{
		if (InPause) GetWorld()->GetTimerManager().PauseTimer(SyncDebugTimerHandle);
		else GetWorld()->GetTimerManager().UnPauseTimer(SyncDebugTimerHandle);
	}
}

void ARhythmNote::OnTakenFromPool_Implementation()
{
	NoteLifeTime = 0.f;
	SpawnMusicTimeSec = 0.0;
	bSyncArrivalLogged = false;
	bIsMoving = true;

	NoteHandle = FNoteHandle();
	NoteHandle.NoteActor = this;

	if (CachedRhythmNoteChannelSubsystem.IsValid())
	{
		CachedRhythmNoteChannelSubsystem->OpenChannel(NoteHandle.Id);
	}
	CancelSyncDebugTimer();
}


void ARhythmNote::OnReturnToPool_Implementation()
{
	NoteLifeTime = 0.f;
	SpawnMusicTimeSec = 0.0;
	bIsMoving = false;

	if (ParentSpawner.IsValid())
	{
		ParentSpawner->RemoveActiveNote(this);
		ParentSpawner = nullptr;
	}

	if (CachedRhythmNoteChannelSubsystem.IsValid())
	{
		CachedRhythmNoteChannelSubsystem->EmitDespawn(NoteHandle.Id);
		CachedRhythmNoteChannelSubsystem->CloseChannel(NoteHandle.Id);
	}
	CancelSyncDebugTimer();
}

void ARhythmNote::InitNote(const ARhythmActor* InRhythmActor, const ARhythmNoteSpawner* InSpawner, const TSubclassOf<ANoteVisualizer>& InNoteVisualizerClass, float InTimeToComplete, const FString& InUserCueName, double InSpawnMusicTimeSec)
{
	checkf(InRhythmActor, TEXT("RhythmActor not Valid in %s"), *GetName());
	checkf(InSpawner, TEXT("Spawner not Valid in %s"), *GetName());
	checkf(InSpawner->GetSpawnerType() != EInstrumentType::Invalid, TEXT("Spawner Type is Invalid"));

	NoteType = InSpawner->GetSpawnerType();
	ParentSpawner = const_cast<ARhythmNoteSpawner*>(InSpawner);
	TimeToComplete = InTimeToComplete;
	SpawnMusicTimeSec = InSpawnMusicTimeSec;

	CachedNoteVisualizerClass = InNoteVisualizerClass;

	if (CachedActorPoolSubsystem.Get() && CachedNoteVisualizerClass)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation());
		SpawnTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
		SpawnTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
		if (ANoteVisualizer* FindJudgementRing = Cast<ANoteVisualizer>(CachedActorPoolSubsystem->Acquire(CachedNoteVisualizerClass, SpawnTransform)))
		{
			CachedNoteVisualizer = FindJudgementRing;
			CachedNoteVisualizer->Init(NoteHandle, NoteType, InRhythmActor->GetFocusedInstrumentType());
		}
	}

	// 판정선은 스포너 전방 1000uu. 월드 +X 고정이 아니라서 회전 배치해도 따라간다
	StartLocation = InSpawner->GetActorLocation();
	EndLocation = StartLocation + InSpawner->GetActorForwardVector() * NoteTravelDistance;

	//이전에 캐릭터 발밑이 아니라 WBP_Rhythm에서 UI를 통해 리듬게임 하던 시절 쓰던 코드
	//URhythmSpawnWidgetBase* RhythmSpawnWidget = InRhythmActor->GetRhythmUIRootWidget()->RhythmSpawnWidget;
	//if (RhythmSpawnWidget)
	//{
	//	URhythmNoteWidgetBase* PooledNoteWidget = RhythmSpawnWidget->SpawnPooledRhythmNoteWidget(NoteType);
	//	if (PooledNoteWidget)
	//	{
	//		PooledNoteWidget->InitWithCueMessage(InUserCueName);
	//	}
	//	if (RhythmNoteUIControllerComponent && RhythmSpawnWidget && PooledNoteWidget)
	//	{
	//		RhythmNoteUIControllerComponent->InitSettings(RhythmSpawnWidget, PooledNoteWidget, NoteHandle);
	//	}
	//}
	
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
	bIsMoving = true;
}

void ARhythmNote::StartSyncDebugTimer(ARhythmActor* RhythmActor, float InDelaySeconds)
{
	bHasSyncDebugTimer = false;

	UWorld* World = GetWorld();
	if (!World) return;

	if (!IsValid(RhythmActor)) return;

	//CachedRhythmActor = RhythmActor;
	//if (CachedRhythmActor.Get())
	//{
	//	if (URhythmSubsystem* Subsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	//	{
	//		Subsystem->OnMusicUserCue.RemoveDynamic(this, &ThisClass::OnMusicUserCueHandler);
	//		Subsystem->OnMusicUserCue.AddDynamic(this, &ThisClass::OnMusicUserCueHandler);
	//	}
	//}
	//

	// 타이머 덮어쓰기 전에 혹시 남아있으면 정리
	World->GetTimerManager().ClearTimer(SyncDebugTimerHandle);

	// 노트 기준으로 “완벽 타이밍에 DetectNotes()를 강제로 한 번 쳐 보는” 타이머
	FTimerDelegate Delegate;
	Delegate.BindWeakLambda(this, [this, RhythmActor]()
		{
			if (!IsValid(this) || !IsValid(RhythmActor)) return;

			// 현재 플레이어가 이 노트의 라인을 보고 있을 때만 자동 판정
			if (RhythmActor->GetFocusedInstrumentType() == NoteType)
			{
				RhythmActor->DetectNotes();
			}
		});

	World->GetTimerManager().SetTimer(
		SyncDebugTimerHandle,
		Delegate,
		InDelaySeconds,
		false
	);

	bHasSyncDebugTimer = true;
}

void ARhythmNote::CancelSyncDebugTimer()
{
	if (!bHasSyncDebugTimer) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SyncDebugTimerHandle);
	}

	bHasSyncDebugTimer = false;
}

void ARhythmNote::OnMusicUserCueHandler(FName CueName)
{
	if (CachedRhythmActor.Get())
	{
		if (CachedRhythmActor->GetFocusedInstrumentType() == NoteType)
		{
			CachedRhythmActor->DetectNotes();
		}
	}
}

void ARhythmNote::SpawnRhythmResultWidget(ENoteResult InNoteResult)
{
	if (RhythmNoteUIControllerComponent)
	{
		RhythmNoteUIControllerComponent->SpawnRhythmResultWidget(InNoteResult);
	}
}

void ARhythmNote::BeginPlay()
{
	Super::BeginPlay();
	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		CachedRhythmNoteChannelSubsystem = RhythmNoteChannelSubsystem;
	}

	if (UActorPoolSubsystem* ActorPoolSubsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
	{
		CachedActorPoolSubsystem = ActorPoolSubsystem;
	}
}

void ARhythmNote::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelSyncDebugTimer();
	Super::EndPlay(EndPlayReason);
}










