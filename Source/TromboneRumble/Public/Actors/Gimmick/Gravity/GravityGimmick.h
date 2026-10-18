// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/EventGimmickBase.h"
#include "GameplayEffectTypes.h"
#include "GravityGimmick.generated.h"

class ACharacter;
class APawn;
class IConsoleVariable;
class UAkAudioEvent;
class UGameplayEffect;
class UMaterialInterface;
class UMaterialParameterCollection;
class UMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/** Level materials read these numbers from the parameter collection. Do not change the values. */
UENUM(BlueprintType)
enum class EGravityState : uint8
{
	Idle = 0,
	Warning = 1,
	Active = 2,
};

UCLASS()
class TROMBONERUMBLE_API AGravityGimmick : public AEventGimmickBase
{
	GENERATED_BODY()

public:

	AGravityGimmick();

protected:

	//~ Begin AEventGimmickBase Interface
	virtual const UEventGimmickConfig& GetEventConfig() const override;
	virtual void OnWarningStarted() override;
	virtual void OnEventPhaseStarted(int32 PhaseIndex) override;
	virtual void OnEventEnded() override;
	//~ End AEventGimmickBase Interface

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

	/** 저중력 동안 모든 플레이어와 들고 있는 물건의 메시에 덧씌우는 테두리 머티리얼. 비워 두면 테두리가 나오지 않는다 */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Visual", meta = (DisplayName = "테두리 머티리얼"))
	TObjectPtr<UMaterialInterface> OutlineMaterial;

	/** 저중력 동안 모든 플레이어의 몸에 붙는 이펙트. 몸통 메시에 붙으므로 메시 표면을 샘플링하는 이펙트를 쓴다. 비워 두면 나오지 않는다 */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Visual", meta = (DisplayName = "캐릭터 이펙트"))
	TObjectPtr<UNiagaraSystem> PlayerVFX;

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

	/** Put the outline and the VFX on the players while the gravity is changed, and take them off after. */
	void UpdatePlayerEffects();

	/**
	 * Put the outline on every mesh of every player and of what they hold, and take it off the meshes they let go of.
	 * Attach the VFX to the body of a player who has none yet.
	 */
	void RefreshPlayerEffects();

	/** Take the outline off every mesh it was put on, and remove every VFX at once. */
	void ClearPlayerEffects();

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

	/** Collects the meshes again while the gravity is changed, so an instrument picked up or dropped follows. */
	FTimerHandle PlayerEffectRefreshTimerHandle;

	/** Meshes that have the outline now. */
	TSet<TWeakObjectPtr<UMeshComponent>> OutlinedMeshes;

	/** VFX attached to each player. One per pawn. */
	TMap<TWeakObjectPtr<APawn>, TWeakObjectPtr<UNiagaraComponent>> PlayerVFXComponents;

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
