// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/Bonus/BonusDrop.h"
#include "DockingPort.generated.h"

class UNiagaraComponent;

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

	/** 착지 후 계속 재생되는 이펙트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> GlowVFXComponent;

	/** 착지 직전에 1회 재생되는 이펙트. 메시 하단에 붙어 포드와 함께 내려온다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> LandingVFXComponent;

	/** 착지 시점 기준 착지 VFX가 시작되는 간격. 착지보다 이만큼 먼저 재생된다 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "DockingPort|VFX", meta = (DisplayName = "착지 VFX 오프셋", ClampMin = "0.0", Units = "s"))
	float LandingVFXOffset = 1.5f;

	/** 착지 시점 기준 반복 VFX가 켜지는 간격. 착지보다 이만큼 뒤에 재생된다 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "DockingPort|VFX", meta = (DisplayName = "반복 VFX 오프셋", ClampMin = "0.0", Units = "s"))
	float GlowVFXOffset = 2.f;

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

	/**
	 * Split the fall into its two parts.
	 *
	 * @param OutCruiseTime Seconds at full speed.
	 * @param OutBrakeTime Seconds of slowing down to the floor.
	 */
	void GetFallTimes(float& OutCruiseTime, float& OutBrakeTime) const;

	/** Turn the landing effect on once. Not on a dedicated server. */
	void PlayLandingVFX();

	/** Turn the glow on. Runs GlowVFXOffset seconds after the landing. */
	void StartGlowVFX();

	/** Location the pod starts to fall from. */
	UPROPERTY(Replicated)
	FVector FallStart = FVector::ZeroVector;

	/** Location the pod stops at. */
	UPROPERTY(Replicated)
	FVector FallEnd = FVector::ZeroVector;

	/** Server time the fall started. Negative until the server sets it. */
	UPROPERTY(Replicated)
	float FallStartServerTime = -1.f;

	/** Has the landing effect played on this machine. It plays before the landing, so the landing must not play it again. */
	bool bLandingVFXPlayed = false;

	FTimerHandle GlowTimerHandle;

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
