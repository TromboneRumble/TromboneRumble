// Copyright (C) 2026 biksari studio. All Rights Reserved.
#include "Actors/Gimmick/PressurePlate/PressurePlate_SpotlightBonus.h"
#include "Actors/Gimmick/PressurePlate/PressurePlateBase.h"
#include "Actors/Gimmick/Spotlight/SpotlightManager.h"
#include "Kismet/GameplayStatics.h"

APressurePlate_SpotlightBonus::APressurePlate_SpotlightBonus()
{
	PrimaryActorTick.bCanEverTick = false;
}


void APressurePlate_SpotlightBonus::BeginPlay()
{
	Super::BeginPlay();
	
}

bool APressurePlate_SpotlightBonus::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return InstigatorActor && InstigatorActor->IsA(APressurePlateBase::StaticClass()) && HasAuthority();
}

void APressurePlate_SpotlightBonus::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;
	SpawnSpotlights();
	if (AActor* Spawner = GetOwner())
	{
		if (Spawner->IsA(APressurePlateBase::StaticClass()))
		{
			Spawner->Destroy();
		}
	}
	Destroy();
}

void APressurePlate_SpotlightBonus::SpawnSpotlights()
{
	if (ASpotlightManager* Manager = Cast<ASpotlightManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASpotlightManager::StaticClass())))
	{
		Manager->Server_TriggerAllSpotlightSpawn();
	}
}




