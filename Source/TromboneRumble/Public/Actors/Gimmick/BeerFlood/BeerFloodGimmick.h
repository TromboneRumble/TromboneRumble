// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "BeerFloodGimmick.generated.h"

class ATromboneCharacterBase;
class UGuideSignalComponent;

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

	/** DEBUG : Trombone.BeerFlood.Debug 1 */
	void DebugDrawGimmickState() const;
	
	UPROPERTY(ReplicatedUsing = OnRep_BeerFloodState)
	EBeerFloodState BeerFloodState = EBeerFloodState::Idle;
	
	/** World Z of the surface. The server tick moves it while the beer rises or drains. */
	UPROPERTY(Replicated)
	float CurrentBeerZ = 0.f;
	
	/** Seconds into the current phase. Only the rising and draining phases use it. */
	float PhaseElapsed = 0.f;
	
	TArray<TWeakObjectPtr<ATromboneCharacterBase>> DrowningCharacters;
	
	FTimerHandle PhaseTimerHandle;
	FTimerHandle CycleTimerHandle;
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
