// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FloatableSubsystem.generated.h"

class UFloatableComponent;

/** UFloatableSubsystem
 *
 * List of floatable components in this world plus the current water level.
 * The flood gimmick pushes the level here. Components that register during a flood get it at once.
 */
UCLASS()
class TROMBONERUMBLE_API UFloatableSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	void Register(UFloatableComponent* Component);
	void Unregister(UFloatableComponent* Component);

	/** New surface height for every component. The first call counts as the flood starting. */
	void SetWaterLevel(float WaterZ);

	/** Flood is over. Every component stops floating. */
	void EndFlood();

	bool IsFloodActive() const { return bFloodActive; }
	int32 GetCount() const { return Components.Num(); }
	int32 GetWetCount() const;

private:

	TArray<TWeakObjectPtr<UFloatableComponent>> Components;

	bool bFloodActive = false;
	float CurrentWaterZ = 0.f;

public:

	//~ Begin UWorldSubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UWorldSubsystem Interface
};
