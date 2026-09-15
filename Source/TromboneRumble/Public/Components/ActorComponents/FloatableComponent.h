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

	/** Upward pull on a fully submerged body. Must beat gravity (980). */
	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "부력 가속도", ClampMin = "0.0"))
	float BuoyancyAccel = 2200.f;

	/** Depth in cm where the pull is at full strength. */
	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "완전 침수 깊이", ClampMin = "1.0"))
	float FullSubmersionDepth = 30.f;

	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "물속 선형 저항", ClampMin = "0.0"))
	float WaterLinearDamping = 2.f;

	UPROPERTY(EditAnywhere, Category = "Float", meta = (DisplayName = "물속 회전 저항", ClampMin = "0.0"))
	float WaterAngularDamping = 2.f;

private:

	UPrimitiveComponent* GetTargetPrimitive() const;

	/** Runs Fn on every body of the primitive. */
	void ForEachBody(UPrimitiveComponent& Primitive, TFunctionRef<void(FBodyInstance*)> Fn) const;

	void ApplyBuoyancyToBody(FBodyInstance* Body) const;

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
