// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/BeerFlood/SequencedBeerBuoyancy.h"
#include "Components/ActorComponents/FloatableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "Subsystems/WorldSubsystem/FloatableSubsystem.h"

namespace
{
	const FName StableUntilWetTag(TEXT("Midterm_StableUntilWet"));
}

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
		// Opt-in props retain their placed pose while the room is dry. Other
		// floatable props keep their existing physics behaviour.
		if (It->ActorHasTag(StableUntilWetTag))
		{
			if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(It->GetRootComponent()))
			{
				Primitive->SetMobility(EComponentMobility::Movable);
				Primitive->SetSimulatePhysics(false);
			}
		}

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
		const float WaterZ = WaterSurface->GetActorLocation().Z;
		Floatables->SetWaterLevel(WaterZ);
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			if (!It->ActorHasTag(StableUntilWetTag)) continue;
			UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(It->GetRootComponent());
			if (!Primitive) continue;

			// Release after water reaches the lower body, rather than releasing
			// into dry furniture contacts when only the feet first get wet.
			const float ReleaseWaterZ = Primitive->Bounds.Origin.Z - Primitive->Bounds.BoxExtent.Z * 0.25f;
			if (WaterZ >= ReleaseWaterZ)
			{
				Primitive->SetSimulatePhysics(true);
				Primitive->WakeAllRigidBodies();
				It->Tags.Remove(StableUntilWetTag);
			}
		}
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
