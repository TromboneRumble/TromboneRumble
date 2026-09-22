// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FloatableComponent.generated.h"

struct FBodyInstance;
class UPrimitiveComponent;

/** UFloatableComponent
 *
 * Floats the owner on the beer flood. Works on any actor with a simulating body.
 * Registers with UFloatableSubsystem in BeginPlay and gets the water level from it on every machine.
 * Characters point it at their mesh and allow it only while ragdolled.
 */
UCLASS(ClassGroup = (Gimmick), meta = (BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UFloatableComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UFloatableComponent();

	/** New surface height. The first call starts ticking. */
	void SetWaterLevel(float InWaterZ);

	/** Flood is over. Puts the damping back and stops ticking. */
	void EndFloating();

	/** Floats this primitive instead of the root. */
	void SetTargetPrimitive(UPrimitiveComponent* InPrimitive) { TargetPrimitive = InPrimitive; }

	/** Off means no force at all. Turning it off also puts the damping back. */
	void SetFloatingAllowed(bool bInAllowed);

	bool IsWet() const { return bWet; }

protected:

	/** Part of the body under the surface at rest. Lower floats higher. Boxy shapes need 0.2 or less to lie flat. */
	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "상대 밀도", ClampMin = "0.05", ClampMax = "0.95"))
	float RelativeDensity = 0.25f;

	/** Depth in cm where a sample point pulls at full strength. 0 uses half of the shortest collision side. */
	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "완전 침수 깊이 (0=자동)", ClampMin = "0.0"))
	float FullSubmersionDepth = 0.f;

	/** Resists up and down motion at each wet corner. Settles bobbing and rocking without slowing drift. */
	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "모서리 수직 저항", ClampMin = "0.0"))
	float CornerDrag = 40.f;

	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "물속 선형 저항", ClampMin = "0.0"))
	float WaterLinearDamping = 2.f;

	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "물속 회전 저항", ClampMin = "0.0"))
	float WaterAngularDamping = 1.f;

	/** Single body only. Splits the pull over the 8 corners of the local bounds so the body can tilt and roll. */
	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "바운즈 모서리 샘플링"))
	bool bSampleBoundsCorners = true;

	/** Spin given once when the body gets wet, in degrees per second. Breaks the symmetry of an upright body. 0 turns it off. */
	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "젖을 때 회전 속도", ClampMin = "0.0"))
	float WetTiltSpeed = 30.f;

private:

	UPrimitiveComponent* GetTargetPrimitive() const;

	/** Runs Fn on every body of the primitive. */
	void ForEachBody(UPrimitiveComponent& Primitive, TFunctionRef<void(FBodyInstance*)> Fn) const;

	/** Gravity divided by the density. Applied to a fully submerged body. */
	float GetBuoyancyAccel() const;

	/** FullSubmersionDepth, or half of the shortest side when it is 0. */
	float GetFullDepth(const FVector& Size) const;

	/** 0 above the water, 1 at FullDepth below it. */
	float GetSubmersion(float Z, float FullDepth) const;

	/** One pull at the body center. Used for skeletal bodies. */
	void ApplyBuoyancyToBody(FBodyInstance* Body) const;

	/** Eight pulls at the collision box corners, each with its own drag. Lower corners pull harder, so the body tilts. */
	void ApplyBuoyancyAtCorners(const UPrimitiveComponent& Primitive, FBodyInstance* Body) const;

	/** Small spin around a flat axis. Seeded by the owner name so server and client agree. */
	void ApplyWetTilt(UPrimitiveComponent& Primitive) const;

	/** Wet bodies use the water damping. The flag stays on until the flood ends or floating is turned off. */
	void SetWet(bool bNewWet, UPrimitiveComponent* Primitive);

	bool bWet = false;
	bool bAllowed = true;
	float WaterZ = 0.f;

	TWeakObjectPtr<UPrimitiveComponent> TargetPrimitive;

	/** Damping before the water, put back when the body dries. */
	float SavedLinearDamping = 0.f;
	float SavedAngularDamping = 0.f;

public:

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface
};
