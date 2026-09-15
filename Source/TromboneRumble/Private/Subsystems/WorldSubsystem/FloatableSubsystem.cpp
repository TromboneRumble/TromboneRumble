// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/WorldSubsystem/FloatableSubsystem.h"
#include "Components/ActorComponents/FloatableComponent.h"

bool UFloatableSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;

	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UFloatableSubsystem::Register(UFloatableComponent* Component)
{
	if (!Component) return;

	Components.AddUnique(Component);

	// Spawned during a flood: no need to wait for the next push
	if (bFloodActive)
	{
		Component->SetWaterLevel(CurrentWaterZ);
	}
}

void UFloatableSubsystem::Unregister(UFloatableComponent* Component)
{
	Components.RemoveSingleSwap(Component);
}

void UFloatableSubsystem::SetWaterLevel(const float WaterZ)
{
	bFloodActive = true;
	CurrentWaterZ = WaterZ;

	for (int32 Index = Components.Num() - 1; Index >= 0; --Index)
	{
		if (UFloatableComponent* Component = Components[Index].Get())
		{
			Component->SetWaterLevel(WaterZ);
		}
		else
		{
			Components.RemoveAtSwap(Index);
		}
	}
}

void UFloatableSubsystem::EndFlood()
{
	bFloodActive = false;

	for (const TWeakObjectPtr<UFloatableComponent>& WeakComponent : Components)
	{
		if (UFloatableComponent* Component = WeakComponent.Get())
		{
			Component->EndFloating();
		}
	}
}

int32 UFloatableSubsystem::GetWetCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<UFloatableComponent>& WeakComponent : Components)
	{
		if (const UFloatableComponent* Component = WeakComponent.Get(); Component && Component->IsWet())
		{
			++Count;
		}
	}
	return Count;
}
