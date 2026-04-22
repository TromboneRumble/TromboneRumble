// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/InstrumentIndicator.h"

#include "Characters/TromboneCharacterBase.h"
#include "Items/InstrumentBase.h"
#include "Components/ActorComponents/FloatingRotatingComponent.h"

AInstrumentIndicator::AInstrumentIndicator()
{
	PrimaryActorTick.bCanEverTick = true;
	FloatingRotatingComponent = CreateDefaultSubobject<UFloatingRotatingComponent>(TEXT("FloatingRotatingComp"));

}

void AInstrumentIndicator::InitInstrument(AInstrumentBase* InInstrumentBase, const FVector& InIndicatorOffset)
{
	if (IsValid(Owner))
	{
		OwnerInstrument = InInstrumentBase;
		IndicatorOffset = InIndicatorOffset;
	}
}

void AInstrumentIndicator::ResetBaseLocation(const FVector& InLocation)
{
	if (FloatingRotatingComponent)
	{
		FloatingRotatingComponent->ResetBaseLocation(InLocation);
	}
}

void AInstrumentIndicator::BeginPlay()
{
	Super::BeginPlay();
	ATromboneCharacterBase::ApplyOccludedStencilToActor(this);
}

void AInstrumentIndicator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (OwnerInstrument.Get())
	{
		ResetBaseLocation(OwnerInstrument->GetActorLocation() + IndicatorOffset);
	}
}

