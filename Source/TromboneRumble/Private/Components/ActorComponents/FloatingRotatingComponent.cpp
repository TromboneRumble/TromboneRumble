// Fill out your copyright notice in the Description page of Project Settings.


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


void UFloatingRotatingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AActor* Owner = GetOwner();
	if (!Owner || !bIsBaseLocationSet) return;

	// Floating (위아래 이동)
	float Time = GetWorld()->GetTimeSeconds();
	float ZOffset = FMath::Sin(Time * FloatSpeed) * FloatHeight;

	// 현재 기준 위치에서 Z축만 변경
	FVector NewLocation = BaseRelativeLocation;
	NewLocation.Z += ZOffset;

	Owner->SetActorRelativeLocation(NewLocation);

	// Rotation (회전)
	FRotator DeltaRotation = FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f);
	Owner->AddActorLocalRotation(DeltaRotation);
	
}

void UFloatingRotatingComponent::ResetBaseLocation()
{
	if (AActor* Owner = GetOwner())
	{
		BaseRelativeLocation = Owner->GetRootComponent()->GetRelativeLocation();
		bIsBaseLocationSet = true;
	}
}

