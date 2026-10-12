// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/Bonus/BonusDrop.h"
#include "DockingPort.generated.h"

class UAkAudioEvent;
class UNiagaraComponent;

/**
 * DockingPort is the supply pod of the space station.
 * It falls straight down at a fixed speed and slows to a stop just above the floor, as if it fires its thrusters.
 *
 * The server fixes the start, the end and the start time, and every machine computes the same position from the server time.
 * So the fall needs no replicated movement and looks the same everywhere.
 * When it is picked up or its lifetime ends, it plays the fall backwards and launches back up to where it started.
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
	virtual void OnExitStarted() override; // Keeps the pod visible and starts the launch.
	virtual void TickExit(float DeltaTime) override;
	virtual float GetExitDuration() const override;
	//~ End ABonusDrop Interface

	/** Body of the pod and root of the actor. */
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

	/** 착지할 때 포드 위치에서 1회 재생되는 사운드. 모든 플레이어가 듣는다 */
	UPROPERTY(EditDefaultsOnly, Category = "DockingPort|Audio", meta = (DisplayName = "착지 사운드"))
	TObjectPtr<UAkAudioEvent> LandingSound;

	/** 착지 시점 기준 착지 사운드가 시작되는 간격. 착지보다 이만큼 먼저 재생된다 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "DockingPort|Audio", meta = (DisplayName = "착지 사운드 오프셋", ClampMin = "0.0", Units = "s"))
	float LandingSoundOffset = 0.f;

	/** 획득하거나 유지 시간이 끝나 위로 발사될 때 포드에 붙어 재생되는 사운드. 모든 플레이어가 듣는다 */
	UPROPERTY(EditDefaultsOnly, Category = "DockingPort|Audio", meta = (DisplayName = "발사 사운드"))
	TObjectPtr<UAkAudioEvent> LaunchSound;

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

	/** Shape of the fall, from the replicated start, end, speed and brake height. */
	struct FFallProfile
	{
		/** Height of the fall in cm. */
		float Distance = 0.f;

		/** Speed before the brake in cm per second. */
		float Speed = 1.f;

		/** Seconds at full speed. */
		float CruiseTime = 0.f;

		/** Seconds of slowing down to the floor. */
		float BrakeTime = 0.f;

		/** @return Seconds of the whole fall. */
		float TotalTime() const { return CruiseTime + BrakeTime; }
	};

	/** @return The shape of the fall. */
	FFallProfile GetFallProfile() const;

	/** Turn the landing effect on once. */
	void PlayLandingVFX();

	/** Play the landing sound once, attached to the pod. */
	void PlayLandingSound();

	/** Turn the glow on. Runs GlowVFXOffset seconds after the landing. */
	void StartGlowVFX();

	/**
	 * Seconds of the fall the launch plays backwards.
	 * The whole fall once the pod has landed, or only the part fallen so far when it is picked up on the way down.
	 */
	float GetFallTimeToReverse() const;

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

	/** Has the landing sound played on this machine. It can play before the landing, so the landing must not play it again. */
	bool bLandingSoundPlayed = false;

	/** Timer that turns the glow on after the landing. */
	FTimerHandle GlowTimerHandle;

	/** Seconds of the fall the launch plays backwards, fixed when the exit starts. */
	float ExitFallTime = 0.f;

	/**
	 * World time of this machine the launch started at.
	 * Not a server time like the fall. The pod is removed a few seconds later, so a client that joins in between has nothing to catch up on.
	 */
	float ExitStartTime = 0.f;

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
