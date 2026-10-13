// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "GameplayEffectTypes.h"
#include "GravityGimmick.generated.h"

class ACharacter;
class IConsoleVariable;
class UAkAudioEvent;
class UGameplayEffect;
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

	/** Called on server and clients when the state changes. Put extra effects here. */
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

	/** 예고가 시작될 때 모든 플레이어 화면에서 재생되는 사운드. 공기가 빠져나가는 소리처럼 예고 시간에 맞춘 소리를 지정한다 */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Audio", meta = (DisplayName = "예고 사운드"))
	TObjectPtr<UAkAudioEvent> WarningSound;

	/**
	 * Wwise game parameter that muffles the whole mix. 0 is clear and 100 is fully muffled.
	 * It rises over the warning, stays up while the gravity is changed, and falls when it ends.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Audio")
	FName MuffleParameterName = TEXT("RTPC_Space");

private:

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

	/** Play the sound of the state that just started, and move the muffle of the mix. */
	void UpdateAudio();

	/**
	 * Move the muffle parameter in a straight line to a value.
	 *
	 * @param Value 0 for clear, 100 for fully muffled.
	 * @param Duration Seconds the move takes. 0 sets it at once.
	 */
	void SetMuffle(float Value, float Duration) const;

	/** Tick runs only while Trombone.Gravity.Debug is on. */
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
