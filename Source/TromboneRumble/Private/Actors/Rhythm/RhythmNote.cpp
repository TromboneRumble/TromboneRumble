// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNote.h"

#include "Actors/Rhythm/RhythmActor.h"
#include "Components/SphereComponent.h"
#include "Components/ActorComponents/RhythmNoteUIControllerComponent.h"
#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
#include "UI/UserWidgets/Rhythm/Note/RhythmNoteWidgetBase.h"
#include "UI/UserWidgets/Rhythm/SpawnWidget/RhythmSpawnWidgetBase.h"
#include "Utilities/DebugHelper.h"

ARhythmNote::ARhythmNote()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	OuterSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OuterSphere"));
	OuterSphere->SetupAttachment(RootComponent);
	InnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InnerSphere"));
	InnerSphere->SetupAttachment(RootComponent);

	RhythmNoteUIControllerComponent = CreateDefaultSubobject<URhythmNoteUIControllerComponent>(TEXT("RhythmNoteUIControllerComponent"));
}

void ARhythmNote::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsMoving) return;
	NoteLifeTime += DeltaTime;
	float Alpha = FMath::Clamp(NoteLifeTime / TimeToComplete, 0.f, 1.f);
	FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	SetActorLocation(NewLocation);
	CachedRhythmNoteChannelSubsystem->UpdateProgress(NoteHandle.Id, Alpha);
}

void ARhythmNote::OnTakenFromPool_Implementation()
{
	NoteLifeTime = 0.f;
	bIsMoving = true;

	NoteHandle = FNoteHandle();
	NoteHandle.NoteActor = this;

	CachedRhythmNoteChannelSubsystem->OpenChannel(NoteHandle.Id);
	CancelSyncDebugTimer();
}


void ARhythmNote::OnReturnToPool_Implementation()
{
	NoteLifeTime = 0.f;
	bIsMoving = false;

	CachedRhythmNoteChannelSubsystem->EmitDespawn(NoteHandle.Id);
	CachedRhythmNoteChannelSubsystem->CloseChannel(NoteHandle.Id);
	CancelSyncDebugTimer();
}

void ARhythmNote::InitNote(const ARhythmActor* InRhythmActor, const ARhythmNoteSpawner* InSpawner,  float InTimeToComplete, const FString& InUserCueName)
{
	checkf(InRhythmActor, TEXT("RhythmActor not Valid in %s"), *GetName());
	checkf(InSpawner, TEXT("Spawner not Valid in %s"), *GetName());
	checkf(InSpawner->GetSpawnerType() != EInstrumentType::Invalid, TEXT("Spawner Type is Invalid"));

	NoteType = InSpawner->GetSpawnerType();
	TimeToComplete = InTimeToComplete;

	StartLocation = InSpawner->GetActorLocation();
	EndLocation = StartLocation + FVector(1000.f, 0.f, 0.f);

	URhythmSpawnWidgetBase* RhythmSpawnWidget = InRhythmActor->GetRhythmUIRootWidget()->RhythmSpawnWidget;
	if (!RhythmSpawnWidget)
	{
		return;
	}
	URhythmNoteWidgetBase* PooledNoteWidget = RhythmSpawnWidget->SpawnPooledRhythmNoteWidget(NoteType);
	if (!PooledNoteWidget)
	{
		return;
	}
	PooledNoteWidget->InitWithCueMessage(InUserCueName);

	if (RhythmNoteUIControllerComponent)
	{
		RhythmNoteUIControllerComponent->InitSettings(RhythmSpawnWidget,PooledNoteWidget, NoteHandle);
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
	bIsMoving = true;
}

void ARhythmNote::StartSyncDebugTimer(ARhythmActor* RhythmActor, float InDelaySeconds)
{
	bHasSyncDebugTimer = false;

	UWorld* World = GetWorld();
	if (!World) return;

	if (!IsValid(RhythmActor)) return;

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
}





