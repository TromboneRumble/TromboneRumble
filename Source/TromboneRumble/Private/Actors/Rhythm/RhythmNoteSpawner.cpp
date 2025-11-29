// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/RhythmNoteSpawner.h"
#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "Components/ArrowComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Actors/Rhythm/RhythmNote.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"
#include "UI/UserWidgets/Rhythm/RhythmUIRootWidget.h"
#include "UI/UserWidgets/Rhythm/RhythmSpawnWidget.h"

ARhythmNoteSpawner::ARhythmNoteSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("NoteSpawner"));

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->SetupAttachment(RootComponent);
		ArrowComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	}

	

}

void ARhythmNoteSpawner::InitSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent, UAkSwitchValue* InChangeSwitch,
	UAkAudioEvent* InFailEvent)
{
	SpawnerType = InType;
	SpawnNoteEvent = InNoteEvent;
	ChangeSwitch = InChangeSwitch;
	FailEvent = InFailEvent;

	ARhythmActor* OwnerRhythmActor = Cast<ARhythmActor>(GetOwner());
	if (IsValid(OwnerRhythmActor))
	{
		CreateSpawnWidget(OwnerRhythmActor);
	}
}

void ARhythmNoteSpawner::OnAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
	{
		const FString CueName = MusicInfo->UserCueName;
		SpawnAndMoveNote(CueName);
	}

}

void ARhythmNoteSpawner::BeginPlay()
{
	Super::BeginPlay();
	CachedActorPoolSubsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>();
	CachedRhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>();
	if (CachedActorPoolSubsystem.Get() && RhythmNoteClass)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation());
		SpawnTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
		SpawnTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
		CachedActorPoolSubsystem->Prewarm(RhythmNoteClass, 100, SpawnTransform);
	}
}

void ARhythmNoteSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnInstrumentPicked.RemoveDynamic(SpawnWidget, &URhythmSpawnWidget::PlayFadeAnimation);
	}
}

void ARhythmNoteSpawner::CreateSpawnWidget(const ARhythmActor* InRhythmActor)
{
	checkf(IsValid(InRhythmActor), TEXT("InRhythmActor is invalid in %s"), *GetName());
	SpawnWidget = CreateWidget<URhythmSpawnWidget>(GetWorld(), RhythmSpawnWidgetClass);
	if (SpawnWidget)
	{
		SpawnWidget->InstrumentType = SpawnerType;
		UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(InRhythmActor->GetRhythmUIRootWidget()->NoteCanvas->AddChild(SpawnWidget));
		NoteSlot->SetAutoSize(true);
		NoteSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		NoteSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		//TODO : Remove Magic Number
		NoteSlot->SetPosition(FVector2D(170.f, 300.f));
		if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnInstrumentPicked.AddDynamic(SpawnWidget, &URhythmSpawnWidget::PlayFadeAnimation);
		}
	}
}

void ARhythmNoteSpawner::SpawnAndMoveNote(const FString& InUserCueName)
{
	checkf(RhythmNoteClass, TEXT("RhythmNoteClass is not set in %s"), *GetName());
	checkf(SpawnWidget, TEXT("SpawnWidget is not created in %s"), *GetName());
	if (!CachedRhythmNoteChannelSubsystem.Get() || !CachedActorPoolSubsystem.Get()) return;

	FString LastChar = InUserCueName.Right(1);
	int32 LineNum = FCString::Atoi(*LastChar);

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(GetActorLocation());
	SpawnTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
	SpawnTransform.SetScale3D(FVector(1.f, 1.f, 1.f));

	//숏노트만 나오게 임시로 설정
	if (!InUserCueName.StartsWith(TEXT("SS_")))
	{
		return;
	}

	if (ARhythmNote* PooledNote = Cast<ARhythmNote>(CachedActorPoolSubsystem->Acquire(RhythmNoteClass, SpawnTransform)))
	{
		URhythmNoteWidget* PooledRhythmNoteWidget = SpawnWidget->GetPooledRhythmNoteWidget(LineNum);
		PooledNote->InitNote(this, PooledRhythmNoteWidget, TimeToComplete, LineNum);

		if (InUserCueName.StartsWith(TEXT("SS_")))
		{
			PooledNote->SetToShortNote();
		}
		else if (InUserCueName.StartsWith(TEXT("LS_")))
		{
			PooledNote->SetToLongNoteStart();
		}
		else if (InUserCueName.StartsWith(TEXT("LC_")))
		{
			//TODO : Lone Note Change 만들기
			return;
		}
		else if (InUserCueName.StartsWith(TEXT("LE_")))
		{
			PooledNote->SetToLongNoteEnd();
		}
		else
		{
			checkf(false, TEXT("Unknown CueName: %s"), *InUserCueName);
		}

		PooledNote->MoveNotes();
	}

	
}
