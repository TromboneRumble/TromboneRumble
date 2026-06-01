// Copyright (C) 2026 biksari studio. All Rights Reserved.


#include "Actors/Gimmick/PressurePlate/PressurePlate_SpawnPresent.h"
#include "Actors/Gimmick/PressurePlate/PressurePlateBase.h"
#include "Actors/Gimmick/Present/Present.h"
#include "Engine/World.h"

APressurePlate_SpawnPresent::APressurePlate_SpawnPresent()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APressurePlate_SpawnPresent::BeginPlay()
{
	Super::BeginPlay();
}

bool APressurePlate_SpawnPresent::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return InstigatorActor && InstigatorActor->IsA(APressurePlateBase::StaticClass()) && HasAuthority();
}

void APressurePlate_SpawnPresent::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	if (InstigatorActor)
	{
		SetActorLocation(InstigatorActor->GetActorLocation());
	}

	SpawnPresent();
	if (AActor* Spawner = GetOwner())
	{
		if (Spawner->IsA(APressurePlateBase::StaticClass()))
		{
			Spawner->Destroy();
		}
	}
	Destroy();
}

void APressurePlate_SpawnPresent::SpawnPresent()
{
	if (!HasAuthority() || !PresentClass) return;

	const FVector PlateLocation = GetActorLocation();
	const FVector SpawnLocation = PlateLocation + FVector(0.f, 0.f, SpawnZOffset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	if (APresent* NewPresent = GetWorld()->SpawnActor<APresent>(PresentClass, SpawnTransform, SpawnParams))
	{
		NewPresent->BonusScore = PresentBonusScore;
	}
}
