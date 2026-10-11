// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "BeerFloodGimmick.generated.h"

class ATromboneCharacterBase;
class UAkAudioEvent;
class UAkComponent;
class UGuideSignalComponent;
class UMaterialParameterCollection;

UENUM(BlueprintType)
enum class EBeerFloodState : uint8
{
	Idle,		// 대기 - 다음 주기까지
	Warning,	// 전조 - 술통이 부풀어 오른다
	Rising,		// 수위 상승
	Sustain,	// 최고 수위 유지
	Draining,	// 배수
};

UCLASS()
class TROMBONERUMBLE_API ABeerFloodGimmick : public AGimmickBase
{
	GENERATED_BODY()

public:
	
	ABeerFloodGimmick();
	
	/** @return The phase the flood is in right now. */
	EBeerFloodState GetBeerFloodState() const { return BeerFloodState; }
	
	/** @return World Z of the beer surface. */
	UFUNCTION(BlueprintPure, Category = "BeerFlood")
	float GetCurrentBeerZ() const { return CurrentBeerZ; }
	
protected:
	
	/** Put the visuals for each phase here. Called on every machine. */
	UFUNCTION(BlueprintImplementableEvent, Category = "BeerFlood")
	void OnBeerFloodStateChanged(EBeerFloodState NewState);
	
protected:
	
	// The settings a designer tunes are in UBeerFloodGimmickConfig

	/** Seconds between two checks for characters below the surface. A technical value, not a design one. */
	UPROPERTY(EditDefaultsOnly, Category = "BeerFlood", meta = (DisplayName = "침수 판정 주기", ClampMin = "0.02"))
	float DrowningCheckInterval = 0.1f;

	/** Shows the guide lines during the warning. */
	UPROPERTY(VisibleAnywhere, Category = "BeerFlood")
	TObjectPtr<UGuideSignalComponent> GuideSignal;

	/** Loop that starts at the barrels when the beer starts to rise. Stopped when the beer reaches its top. */
	UPROPERTY(EditDefaultsOnly, Category = "BeerFlood|Sound", meta = (DisplayName = "물줄기 시작"))
	TObjectPtr<UAkAudioEvent> PourStartEvent;

	/** Stops the pour loop. Also posted when the gimmick turns off, so the loop never stays on. */
	UPROPERTY(EditDefaultsOnly, Category = "BeerFlood|Sound", meta = (DisplayName = "물줄기 정지"))
	TObjectPtr<UAkAudioEvent> PourStopEvent;

	/** Plays without a position when the beer starts to rise. */
	UPROPERTY(EditDefaultsOnly, Category = "BeerFlood|Sound", meta = (DisplayName = "수위 상승"))
	TObjectPtr<UAkAudioEvent> WaterRiseEvent;

	/** Plays without a position when the beer starts to drain. */
	UPROPERTY(EditDefaultsOnly, Category = "BeerFlood|Sound", meta = (DisplayName = "수위 하강"))
	TObjectPtr<UAkAudioEvent> WaterDrainEvent;

	/** Where the pour loop plays, relative to the actor. Drag the widget to the barrels in the level. */
	UPROPERTY(EditAnywhere, Category = "BeerFlood|Sound", meta = (DisplayName = "물줄기 사운드 위치", MakeEditWidget))
	FVector PourSoundOffset = FVector::ZeroVector;

	/** Collection the barrel materials read. The state is written as 0 to 4 and the progress of the phase as 0 to 1. */
	UPROPERTY(EditDefaultsOnly, Category = "BeerFlood|Visual")
	TObjectPtr<UMaterialParameterCollection> GimmickParameterCollection;

	UPROPERTY(EditDefaultsOnly, Category = "BeerFlood|Visual")
	FName StateParameterName = TEXT("BeerFloodState");

	UPROPERTY(EditDefaultsOnly, Category = "BeerFlood|Visual")
	FName ProgressParameterName = TEXT("BeerFloodProgress");

private:
	
	/** @return World Z the surface rests at. The actor's own height. */
	float GetBaseBeerZ() const;
	
	/** @return World Z the surface rises to. */
	float GetPeakBeerZ() const;
	
	void SetBeerFloodState(EBeerFloodState NewState);
	
	UFUNCTION()
	void OnRep_BeerFloodState();
	
	/** One per phase. Each timer starts the next phase. */
	void BeginWarning();
	void BeginRising();
	void BeginSustain();
	void BeginDraining();
	void EndBeerFlood();
	
	/** Knocks down anyone standing below the surface. Skips the ones already down. */
	void UpdateDrowning();
	
	/** Lets every drowning character get up again. */
	void ReleaseAllDrowning();

	/** Sends the surface Z to the floatable subsystem. Every machine. */
	void PushWaterLevel();

	/** Tells the floatable subsystem the flood is over. Every machine. */
	void EndFlood();

	/** @return Seconds the current phase lasts, from the config. 0 while idle. */
	float GetPhaseDuration() const;

	/** @return How far the current phase is, 0 to 1. */
	float GetPhaseProgress() const;

	/** Writes the state and the progress to the parameter collection. Every machine but a dedicated server. */
	void WriteMaterialParameters() const;

	/** Posts the sounds of the phase just entered. Every machine but a dedicated server. */
	void PlayPhaseSounds();

	/** Posts the pour loop at PourSoundOffset. The component is made on first use. */
	void StartPourSound();

	void StopPourSound();

	/** DEBUG : Trombone.BeerFlood.Debug 1 */
	void DebugDrawGimmickState() const;
	
	UPROPERTY(ReplicatedUsing = OnRep_BeerFloodState)
	EBeerFloodState BeerFloodState = EBeerFloodState::Idle;
	
	/** World Z of the surface. The server tick moves it while the beer rises or drains. */
	UPROPERTY(Replicated)
	float CurrentBeerZ = 0.f;
	
	/** Seconds into the current phase. Every machine counts it, so the material progress needs no replication. */
	float PhaseElapsed = 0.f;

	/** Plays the pour loop. Spawned in the world at first use and destroyed with the gimmick. */
	UPROPERTY(Transient)
	TObjectPtr<UAkComponent> PourAkComponent;
	
	TArray<TWeakObjectPtr<ATromboneCharacterBase>> DrowningCharacters;
	
	FTimerHandle PhaseTimerHandle;
	FTimerHandle DrowningTimerHandle;
	
public:
	
	//~ Begin AGimmickBase Interface
	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void ForceTrigger() override;
	//~ End AGimmickBase Interface
	
	//~ Begin AActor Interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface
	
protected:
	
	//~ Begin AActor Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
	
};
