// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/PressurePlate/PressurePlateBase.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Interfaces/Interactable.h"
#include "Net/UnrealNetwork.h"


APressurePlateBase::APressurePlateBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	Frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Frame"));
	RootComponent = Frame;
	Frame->SetIsReplicated(false);

	Platform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	Platform->SetupAttachment(RootComponent);

	PressTargetLocation = CreateDefaultSubobject<USceneComponent>(TEXT("PressTargetLocation"));
	PressTargetLocation->SetupAttachment(RootComponent);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(50.f, 50.f, 30.f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

	PressureTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("PressureTimeline"));
}

void APressurePlateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APressurePlateBase, bIsPressed);
}


void APressurePlateBase::BeginPlay()
{
	Super::BeginPlay();

	if (Platform)
	{
		InitialLocation = Platform->GetRelativeLocation();
	}

	if (HasAuthority() && TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnOverlapEnd);
	}
	

	// 타임라인 바인딩
	if (PressureCurve)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("UpdatePlateLocation"));
		PressureTimeline->AddInterpFloat(PressureCurve, TimelineProgress);

		FOnTimelineEvent TimelineFinished;
		TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));
		PressureTimeline->SetTimelineFinishedFunc(TimelineFinished);
	}
}

void APressurePlateBase::OnRep_IsPressed()
{
	if (!PressureTimeline) return;

	if (bIsPressed)
	{
		PressureTimeline->Play();
	}
	else
	{
		PressureTimeline->Reverse();
	}
}

void APressurePlateBase::OnTimelineFinished()
{
	if (bIsPressed && PressureTimeline->GetPlaybackPosition() >= PressureTimeline->GetTimelineLength())
	{
		if (HasAuthority())
		{
			Server_OnPlateActivated();
		}
	}
}

void APressurePlateBase::Server_OnPlateActivated()
{
	for (AActor* TargetActor : LinkedActors)
	{
		if (!IsValid(TargetActor))
		{
			continue;
		}

		if (TargetActor->Implements<UInteractable>())
		{
			bool bCanInteract = IInteractable::Execute_CanInteract(TargetActor, this);

			if (bCanInteract)
			{
				IInteractable::Execute_Interact(TargetActor, this);
			}
		}
	}
}

void APressurePlateBase::UpdatePlateLocation(const float InAlpha)
{
	if (!Platform || !PressTargetLocation) return;

	
	FVector TargetRelativeLoc = PressTargetLocation->GetRelativeLocation();
	FVector NewLocation = FMath::Lerp(InitialLocation, TargetRelativeLoc, InAlpha);

	Platform->SetRelativeLocation(NewLocation);
}

void APressurePlateBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if (OtherActor && OtherActor != this && !OtherActor->IsA(APressurePlateBase::StaticClass()))
	{
		OverlappingCount++;
		if (OverlappingCount > 0 && !bIsPressed)
		{
			bIsPressed = true;
			OnRep_IsPressed();
		}
	}
}

void APressurePlateBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	if (OtherActor && OtherActor != this)
	{
		OverlappingCount = FMath::Max(0, OverlappingCount - 1);

		// 아무도 없게 되었는데, 눌린 상태라면
		if (OverlappingCount == 0 && bIsPressed)
		{
			bIsPressed = false;
			OnRep_IsPressed();
		}
	}
}



