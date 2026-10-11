// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/FloatingRotatingComponent.h"

UFloatingRotatingComponent::UFloatingRotatingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFloatingRotatingComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetBaseLocation();
}

void UFloatingRotatingComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner || !bIsBaseLocationSet) return;

	// The world time drives the bob, so a pause or a hitch never throws the wave off
	FVector NewLocation = BaseRelativeLocation;
	NewLocation.Z += FMath::Sin(GetWorld()->GetTimeSeconds() * FloatSpeed) * FloatHeight;
	Owner->SetActorRelativeLocation(NewLocation);

	Owner->AddActorLocalRotation(FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f));
}

void UFloatingRotatingComponent::ResetBaseLocation()
{
	const AActor* Owner = GetOwner();
	const USceneComponent* Root = Owner ? Owner->GetRootComponent() : nullptr;
	if (!Root) return;

	ResetBaseLocation(Root->GetRelativeLocation());
}

void UFloatingRotatingComponent::ResetBaseLocation(const FVector& InLocation)
{
	BaseRelativeLocation = InLocation;
	bIsBaseLocationSet = true;
}
