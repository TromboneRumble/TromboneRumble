// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/NoteVisualizer.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "Components/StaticMeshComponents/RingHitBoxComponent.h"
#include "Kismet/GameplayStatics.h"

ANoteVisualizer::ANoteVisualizer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	if (RingMesh)
	{
		SetRootComponent(RingMesh);
		RingMesh->SetCastShadow(false);
		RingMesh->bReceivesDecals = false;
		RingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RingMesh->SetSimulatePhysics(false);
		RingMesh->SetGenerateOverlapEvents(false);
	}
	

}

void ANoteVisualizer::Init(const FNoteHandle& InNoteHandle, const EInstrumentType& InType, const EInstrumentType& HeldType)
{
	NoteHandle = InNoteHandle;
	InstrumentType = InType;
	if (InType == HeldType && EInstrumentType::Background < InstrumentType && InstrumentType < EInstrumentType::None && RingMesh)
	{
		ShowRing(true);
	}
	else
	{
		ShowRing(false);
	}
	const bool bNeedAttach =
		!CachedRingHitBoxComponent.IsValid()
		|| !GetRootComponent()
		|| GetRootComponent()->GetAttachParent() != CachedRingHitBoxComponent.Get();

	if (bNeedAttach)
	{
		FindPlayerCharacterAndAttach();
	}

	UnBindChannel();
	BindChannel();
}



void ANoteVisualizer::OnTakenFromPool_Implementation()
{
	IPoolable::OnTakenFromPool_Implementation();
	EnsureMID();
	SizeAlpha = 0.f;	
}

void ANoteVisualizer::OnReturnToPool_Implementation()
{
	IPoolable::OnReturnToPool_Implementation();
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CachedRingHitBoxComponent.Reset();
	ShowRing(false);
}

void ANoteVisualizer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureMID();
	ApplyMaterialParams();
}

void ANoteVisualizer::BeginPlay()
{
	Super::BeginPlay();
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::HandleOnInstrumentPicked);
	}
	EnsureMID();
	ApplyMaterialParams();
}

void ANoteVisualizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnBindChannel();
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnInstrumentPicked.RemoveDynamic(this, &ThisClass::HandleOnInstrumentPicked);
	}
	Super::EndPlay(EndPlayReason);
}


void ANoteVisualizer::SetAlpha(float InAlpha)
{
	SizeAlpha = FMath::Clamp(InAlpha, 0.0f, 1.0f);

	EnsureMID();
	if (MID)
	{
		MID->SetScalarParameterValue(ParamName_SizeAlpha, SizeAlpha);
	}
}

void ANoteVisualizer::ApplyMaterialParams()
{
	if (!MID) return;

	MID->SetScalarParameterValue(ParamName_SizeAlpha, SizeAlpha);
}

void ANoteVisualizer::BindChannel()
{
	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		if (FNoteChannel* Channel = RhythmNoteChannelSubsystem->GetChannelById(NoteHandle.Id))
		{
			ProgressHandle = Channel->OnProgress.AddWeakLambda(this, [this](float Alpha)
				{
					SetAlpha(Alpha);
				});
			DespawnHandle = Channel->OnDespawn.AddWeakLambda(this, [this]()
				{
					ShowRing(false);
					if (UActorPoolSubsystem* Subsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
					{
						Subsystem->Release(this);
					}
				});
		}
	}
}

void ANoteVisualizer::UnBindChannel()
{
	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		if (FNoteChannel* Channel = RhythmNoteChannelSubsystem->GetChannelById(NoteHandle.Id))
		{
			if (ProgressHandle.IsValid())
			{
				Channel->OnProgress.Remove(ProgressHandle);
				ProgressHandle.Reset();
			}
			if (DespawnHandle.IsValid())
			{
				Channel->OnDespawn.Remove(DespawnHandle);
				DespawnHandle.Reset();
			}
		}
	}
}

void ANoteVisualizer::ShowRing(bool bShow)
{
	if (!RingMesh) return;
	if (bShow)
	{
		RingMesh->SetHiddenInGame(false, true);
		RingMesh->SetVisibility(true, true);
	}
	else
	{
		RingMesh->SetHiddenInGame(true, true);
		RingMesh->SetVisibility(false, true);
	}

}

void ANoteVisualizer::FindPlayerCharacterAndAttach()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}

	URingHitBoxComponent* RingHitBox =
		Pawn->FindComponentByClass<URingHitBoxComponent>();

	if (!RingHitBox)
	{
		return;
	}

	AttachToComponent(
		RingHitBox,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);
	SetActorRelativeLocation(FVector(0.0f, 0.0f, 2.0f));
	CachedRingHitBoxComponent = RingHitBox;
}

void ANoteVisualizer::HandleOnInstrumentPicked(EInstrumentType OldType, EInstrumentType NewType)
{
	if (!RingMesh) return;
	if (NewType == InstrumentType)
	{
		ShowRing(true);
	}
	else
	{
		ShowRing(false);
	}
}

void ANoteVisualizer::EnsureMID()
{
	if (!RingMesh) return;

	// 이미 만들어져 있으면 재사용
	if (MID) return;

	// Element 0에 이미 Material/MI가 들어있으면 Source Material 안 넣어도 됨
	MID = RingMesh->CreateDynamicMaterialInstance(0);
}
