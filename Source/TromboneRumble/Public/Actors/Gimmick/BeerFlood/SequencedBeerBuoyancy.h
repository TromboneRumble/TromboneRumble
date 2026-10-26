// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SequencedBeerBuoyancy.generated.h"

/** Sends a Sequencer-controlled surface's actual height to the existing prop buoyancy. */
UCLASS()
class TROMBONERUMBLE_API ASequencedBeerBuoyancy : public AActor
{
	GENERATED_BODY()

public:
	ASequencedBeerBuoyancy();
	virtual void Tick(float DeltaSeconds) override;

	/** The visible surface animated by Sequencer. */
	UPROPERTY(EditInstanceOnly, Category = "Beer Buoyancy")
	TObjectPtr<AActor> WaterSurface;

	/** Surface Z when the room is dry. Buoyancy ends when the surface returns here. */
	UPROPERTY(EditInstanceOnly, Category = "Beer Buoyancy", meta = (Units = "cm"))
	float DrySurfaceZ = -3.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool bSupplyingWater = false;
};
