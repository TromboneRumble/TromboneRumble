// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Blizzard/BlizzardShelter.h"
#include "Actors/Gimmick/Blizzard/BlizzardGimmick.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"

ABlizzardShelter::ABlizzardShelter()
{
	PrimaryActorTick.bCanEverTick = false;

	SafeZone = CreateDefaultSubobject<USphereComponent>(TEXT("SafeZone"));
	SetRootComponent(SafeZone);

	// 판정은 거리 계산으로 하므로 물리 콜리전은 불필요. sphere 는 반경 저작/기즈모용.
	SafeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SafeZone->SetSphereRadius(400.f);

	ShelterLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShelterLight"));
	ShelterLight->SetupAttachment(SafeZone);
}

bool ABlizzardShelter::IsLocationInsideShelter(const FVector& WorldLoc) const
{
	if (!SafeZone) return false;

	const float Radius = SafeZone->GetScaledSphereRadius();
	return FVector::DistSquared(SafeZone->GetComponentLocation(), WorldLoc) <= FMath::Square(Radius);
}

void ABlizzardShelter::HandleBlizzardState(EBlizzardState NewState)
{
	if (!ShelterLight) return;

	switch (NewState)
	{
	case EBlizzardState::Warning:	TargetIntensity = WarningIntensity;	break;
	case EBlizzardState::Active:	TargetIntensity = ActiveIntensity;	break;
	case EBlizzardState::Idle:
	default:						TargetIntensity = IdleIntensity;	break;
	}

	// 이전 페이드가 끝나기 전에 상태가 또 바뀔 수 있으므로, 목표가 아니라 "현재 실값"에서 이어 간다.
	StartIntensity = ShelterLight->Intensity;

	OnLightFadeRequested();
}

void ABlizzardShelter::UpdateLightFade(float Alpha)
{
	if (!ShelterLight) return;

	ShelterLight->SetIntensity(FMath::Lerp(StartIntensity, TargetIntensity, Alpha));
}
