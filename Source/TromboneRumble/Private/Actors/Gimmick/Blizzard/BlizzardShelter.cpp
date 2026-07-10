// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Blizzard/BlizzardShelter.h"
#include "Components/SphereComponent.h"

ABlizzardShelter::ABlizzardShelter()
{
	PrimaryActorTick.bCanEverTick = false;

	SafeZone = CreateDefaultSubobject<USphereComponent>(TEXT("SafeZone"));
	SetRootComponent(SafeZone);

	// 판정은 거리 계산으로 하므로 물리 콜리전은 불필요. sphere 는 반경 저작/기즈모용.
	SafeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SafeZone->SetSphereRadius(400.f);
}

bool ABlizzardShelter::IsLocationInside(const FVector& WorldLoc) const
{
	if (!SafeZone) return false;

	const float Radius = SafeZone->GetScaledSphereRadius();
	return FVector::DistSquared(SafeZone->GetComponentLocation(), WorldLoc) <= FMath::Square(Radius);
}
