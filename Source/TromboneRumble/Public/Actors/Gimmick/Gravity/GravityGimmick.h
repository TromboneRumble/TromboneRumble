// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameplayEffectTypes.h"
#include "GravityGimmick.generated.h"

class ACharacter;
class IConsoleVariable;
class UGameplayEffect;
class ULightComponent;
class UMaterialParameterCollection;

/** Level materials read these numbers from the parameter collection. Do not change the values. */
UENUM(BlueprintType)
enum class EGravityState : uint8
{
	Idle = 0,
	Warning = 1,
	Active = 2,
};

UCLASS()
class TROMBONERUMBLE_API AGravityGimmick : public AGimmickBase
{
	GENERATED_BODY()

public:

	AGravityGimmick();

	//~ Begin AGimmickBase Interface
	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void ForceTrigger() override;
	//~ End AGimmickBase Interface

protected:

	/** Called on server and clients when the state changes. Put sound and particles here. Warning lights use the collection values instead. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gimmick")
	void OnGravityStateChanged(EGravityState NewState, float Multiplier);

	// The settings a designer tunes are in UGravityGimmickConfig. Only references stay here

	/** Infinite effect on the GravityScale attribute. Its magnitude is Set By Caller with the Trombone.Gimmick.Gravity.Scale tag. The gimmick removes it. */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|GAS")
	TSubclassOf<UGameplayEffect> GravityEffectClass;

	/** Collection the level materials read. The state is written as 0, 1, 2 and the multiplier as is. */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Visual")
	TObjectPtr<UMaterialParameterCollection> GimmickParameterCollection;

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Visual")
	FName StateParameterName = TEXT("GravityState");

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Visual")
	FName MultiplierParameterName = TEXT("GravityMultiplier");

	/** Collection value that follows the warning lights from 0 to 1, for effects such as the red flash of the screen. */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Visual")
	FName WarningPulseParameterName = TEXT("GravityWarningPulse");

	/**
	 * Actor tag of the level lights that flicker during the warning.
	 * The intensity a light is placed with is its brightest point, and the light stays off outside the warning.
	 * Keep in mind that a light must be Movable or Stationary, because a Static light is baked and cannot change.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Visual")
	FName WarningLightTag = TEXT("GravityWarning");

private:

	/** A level light found by WarningLightTag, with the intensity it was placed with. */
	struct FWarningLight
	{
		TWeakObjectPtr<ULightComponent> Light;
		float BaseIntensity = 0.f;
	};

	/** Start the warning, then the event. The sequence of the stage data calls it through ForceTrigger. Server only. */
	void StartWarning();
	void StartActive();
	void EndActive();

	/** Server only. Sets the state and runs the local reaction at once for the listen host. */
	void SetState(EGravityState NewState);

	UFUNCTION()
	void OnRep_State();

	/** Runs on every machine after the state changes. Writes the collection values and calls the blueprint event. */
	void HandleStateChanged();

	void ApplyEffectToAllPlayers();
	void RemoveAllEffects();

	/** Find the lights with WarningLightTag in the level. Runs on every machine except a dedicated server. */
	void CollectWarningLights();

	/** Turn the warning lights on when the warning starts, and start their fade out when it ends. */
	void RefreshWarningLights();

	/** Move the intensity of the warning lights along the sine wave. Runs every frame during the warning. */
	void UpdateWarningLightFlicker();

	/** Dim the warning lights toward 0, and turn them off at the end. Runs every frame after the warning. */
	void UpdateWarningLightFadeOut();

	/** Set every warning light to this part of the intensity it was placed with, and write the same value as the warning pulse. */
	void SetWarningLightRatio(float Ratio);

	/** Turn the warning lights off at once, give them back the intensity they were placed with, and set the warning pulse to 0. */
	void TurnOffWarningLights();

	/** Write the warning pulse to the collection. */
	void SetWarningPulse(float Pulse);

	/** @return Whether the warning shows anything: a warning light, or the warning pulse of the collection. */
	bool HasWarningVisuals() const;

	/** Tick runs only while the warning visuals are on or Trombone.Gravity.Debug is on. */
	void UpdateTickEnabled();

	/** Read Trombone.Gravity.Debug again when it changes. */
	void HandleDebugCVarChanged(IConsoleVariable* Variable);

	/** One line on screen: state, time left, affected players, local gravity scale. */
	void DebugDraw() const;

	UPROPERTY(ReplicatedUsing = OnRep_State)
	EGravityState State = EGravityState::Idle;

	/** One effect handle per player. Removing by handle leaves other effects on the same attribute alone. */
	TMap<TWeakObjectPtr<ACharacter>, FActiveGameplayEffectHandle> ActiveEffects;

	FTimerHandle PhaseTimerHandle;

	/** Lights that flicker during the warning. Empty on a dedicated server. */
	TArray<FWarningLight> WarningLights;

	/** Local world time the current warning started. The sine wave of the lights starts here. */
	float WarningStartTime = 0.f;

	/** Local world time the fade out started. Negative while the lights do not fade. */
	float FadeOutStartTime = -1.f;

	/** Ratio the fade out starts from, so it continues from wherever the flicker was. */
	float FadeOutStartRatio = 1.f;

	/** Ratio last applied to the warning lights. */
	float CurrentWarningLightRatio = 1.f;

	/** Are the warning visuals on, flickering or fading out. */
	bool bWarningLightsOn = false;

	/** Is Trombone.Gravity.Debug on. */
	bool bDebugDraw = false;

public:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface
};
