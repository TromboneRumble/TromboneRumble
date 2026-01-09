// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/InstrumentIndicator.h"

#include "Components/ActorComponents/FloatingRotatingComponent.h"

AInstrumentIndicator::AInstrumentIndicator()
{
	PrimaryActorTick.bCanEverTick = true;
	FloatingRotatingComponent = CreateDefaultSubobject<UFloatingRotatingComponent>(TEXT("FloatingRotatingComp"));

}

void AInstrumentIndicator::ResetBaseLocation(const FVector& InLocation)
{
	if (FloatingRotatingComponent)
	{
		FloatingRotatingComponent->ResetBaseLocation(InLocation);
	}
}

