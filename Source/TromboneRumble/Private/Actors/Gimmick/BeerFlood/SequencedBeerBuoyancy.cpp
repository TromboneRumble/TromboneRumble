// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/BeerFlood/SequencedBeerBuoyancy.h"
#include "Components/ActorComponents/FloatableComponent.h"
#include "EngineUtils.h"
#include "Subsystems/WorldSubsystem/FloatableSubsystem.h"

ASequencedBeerBuoyancy::ASequencedBeerBuoyancy()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void ASequencedBeerBuoyancy::BeginPlay()
{
	Super::BeginPlay();

	// Supply the height before the existing components apply their physics forces.
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		TInlineComponentArray<UFloatableComponent*> Floatables;
		It->GetComponents(Floatables);
		for (UFloatableComponent* Component : Floatables)
		{
			Component->AddTickPrerequisiteActor(this);
		}
	}
}

void ASequencedBeerBuoyancy::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UFloatableSubsystem* Floatables = GetWorld()->GetSubsystem<UFloatableSubsystem>();
	if (!Floatables) return;

	if (IsValid(WaterSurface) && WaterSurface->GetActorLocation().Z > DrySurfaceZ + 0.01f)
	{
		Floatables->SetWaterLevel(WaterSurface->GetActorLocation().Z);
		bSupplyingWater = true;
	}
	else if (bSupplyingWater)
	{
		Floatables->EndFlood();
		bSupplyingWater = false;
	}
}

void ASequencedBeerBuoyancy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bSupplyingWater)
	{
		if (UFloatableSubsystem* Floatables = GetWorld()->GetSubsystem<UFloatableSubsystem>())
		{
			Floatables->EndFlood();
		}
	}
	Super::EndPlay(EndPlayReason);
}
