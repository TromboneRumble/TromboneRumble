// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/Bonus/BonusDrop.h"
#include "DockingPort.generated.h"

/**
 * DockingPort is the supply pod of the space station.
 * It falls straight down at a fixed speed and slows to a stop just above the floor, as if it fires its thrusters.
 *
 * The server fixes the start, the end and the start time, and every machine computes the same position from the server time.
 * So the fall needs no replicated movement and looks the same everywhere.
 *
 * @see ABonusSpawner
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API ADockingPort : public ABonusDrop
{
	GENERATED_BODY()

public:

	ADockingPort();

	//~ Begin ABonusDrop Interface
	virtual void ApplyConfig(const UBonusGimmickConfig& Config) override;
	//~ End ABonusDrop Interface

protected:

	//~ Begin ABonusDrop Interface
	virtual void TickFalling(float DeltaTime) override;
	virtual void OnLanded() override;
	//~ End ABonusDrop Interface

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PortMesh;

	/** 바닥에서 메시 피벗까지의 높이. 피벗이 바닥면에 있으면 0 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "DockingPort", meta = (DisplayName = "착지 높이 보정", Units = "cm"))
	float LandOffsetZ = 0.f;

private:

	/**
	 * Find where the pod is after it has fallen for Elapsed seconds.
	 *
	 * @param OutLocation Location at that time.
	 * @return Whether the pod has reached the floor.
	 */
	bool EvaluateFall(float Elapsed, FVector& OutLocation) const;

	/** Location the pod starts to fall from. */
	UPROPERTY(Replicated)
	FVector FallStart = FVector::ZeroVector;

	/** Location the pod stops at. */
	UPROPERTY(Replicated)
	FVector FallEnd = FVector::ZeroVector;

	/** Server time the fall started. Negative until the server sets it. */
	UPROPERTY(Replicated)
	float FallStartServerTime = -1.f;

	/** Speed before the brake, from UDockingPortGimmickConfig. Replicated so every machine computes the same fall. */
	UPROPERTY(Replicated)
	float FallSpeed = 1000.f;

	/** Height above the floor the brake starts at, from UDockingPortGimmickConfig. */
	UPROPERTY(Replicated)
	float BrakeHeight = 400.f;

public:

	//~ Begin AActor Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

protected:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface
};
