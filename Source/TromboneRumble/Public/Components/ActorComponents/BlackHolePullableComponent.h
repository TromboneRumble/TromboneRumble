// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/BlackHole/BlackHoleGimmick.h"
#include "Components/ActorComponent.h"
#include "BlackHolePullableComponent.generated.h"

class UPrimitiveComponent;

/** UBlackHolePullableComponent
 *
 * Lets one placed prop be pulled by the black hole. A prop without this component is never touched,
 * which is how fixed scenery and movable props are told apart - no tag and no collision channel.
 *
 * Finds the black hole of the level on its own and subscribes to it, so the gimmick keeps no list of props.
 *
 * Server only. Every force and impulse comes from the server and the owner replicates its movement,
 * so the owner needs a simulating body: Movable, Simulate Physics on, Replicates and Replicate Movement.
 */
UCLASS(ClassGroup = (Gimmick), meta = (BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UBlackHolePullableComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UBlackHolePullableComponent();

	/** @return Is this prop riding the ring, which lasts until the hole collapses. */
	bool IsCaptured() const { return bCaptured; }

protected:

	/** Multiplies the reference speed of the settings for this prop. Below 1 lags behind, above 1 flies in. */
	UPROPERTY(EditAnywhere, Category = "BlackHole", meta = (DisplayName = "끌림 배율", ClampMin = "0.0"))
	float PullScale = 1.f;

	/** Height in cm the prop goes back to its placed spot from. 0 uses KillZ, so it returns instead of being destroyed. */
	UPROPERTY(EditAnywhere, Category = "BlackHole", meta = (DisplayName = "복귀 높이 (0=KillZ 자동)"))
	float ResetZ = 0.f;

	/** Seconds between two fall checks. */
	UPROPERTY(EditAnywhere, Category = "BlackHole", meta = (DisplayName = "복귀 검사 주기", ClampMin = "0.1", Units = "s"))
	float ResetCheckInterval = 1.f;

private:

	/** Body the forces go to, which is the root of the owner. */
	UPrimitiveComponent* GetTargetPrimitive() const;

	/** Pull starts and stops with the state of the hole. */
	UFUNCTION()
	void HandleBlackHoleStateChanged(EBlackHoleState NewState);

	/** Throws the prop out, and only when it was captured. */
	UFUNCTION()
	void HandleBlackHoleBurst(FVector Center);

	/** Wake the body, take its damping off and start ticking. */
	void StartPull();

	/** Put the damping and the gravity back and stop ticking. Must stay safe to call twice. */
	void StopPull();

	/** Hold this prop on the ring: give it a slot and turn its gravity off. */
	void Capture();

	/** Put the prop back where it was placed once it falls out of the map. */
	void CheckFallReset();

	/** ResetZ, or a little above KillZ when it is 0. */
	float GetResetZ() const;

	/** Black hole of the level, found in BeginPlay. */
	TWeakObjectPtr<ABlackHoleGimmick> Gimmick;

	/** Place on the ring, valid while captured. */
	FBlackHoleRingSlot RingSlot;

	bool bPulling = false;
	bool bCaptured = false;

	/** Physics values before the pull, put back when it ends. Gravity can be off by design on a space station. */
	float SavedLinearDamping = 0.f;
	float SavedAngularDamping = 0.f;
	bool bSavedGravityEnabled = true;

	/** Where the prop was placed. The burst can throw it out of the map. */
	FTransform SpawnTransform;

	FTimerHandle ResetTimerHandle;

public:

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface
};
