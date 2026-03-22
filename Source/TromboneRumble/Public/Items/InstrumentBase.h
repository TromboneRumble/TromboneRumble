// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/WeaponBase.h"
#include "Utilities/Defines.h" 
#include "GameplayEffectTypes.h"
#include "Data/InstrumentScoreData.h"
#include "InstrumentBase.generated.h"

class URhythmComboWidgetBase;
class UOSI_WidgetBase;
class AInstrumentIndicator;
class UInstrumentScoreData;
class ADefaultPlayerState;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstrumentBuffStateChanged, bool, bIsActive);

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API AInstrumentBase : public AWeaponBase
{
	GENERATED_BODY()

public:
	AInstrumentBase();

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnInstrumentBuffStateChanged OnBuffStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Client_OnHitSuccess_Implementation(AActor* HitActor) override;
	virtual void OnRep_CurrentOwner(AActor* OldActor) override;
	virtual void Unequip(AActor* OwnerActor) override;

	// Indicator
	UPROPERTY(EditDefaultsOnly, Category = "Config|Indicator")
	TSubclassOf<AInstrumentIndicator> IndicatorClass;

	UPROPERTY(Transient) 
	TObjectPtr<AInstrumentIndicator> IndicatorInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Indicator")
	FVector IndicatorOffset = FVector(0.0f, 0.0f, 100.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Config|Indicator|UI")
	TSubclassOf<UOSI_WidgetBase> IndicatorWidgetClass;

	UPROPERTY()
	TWeakObjectPtr<UOSI_WidgetBase> IndicatorWidgetInstance = nullptr;

	UFUNCTION()
	void TryUpdateIndicatorVisibility();

	FTimerHandle IndicatorRetryTimerHandle;

	UFUNCTION()
	void HandleInGameStateChanged(EInGameState InGameState);

	UFUNCTION()
	void HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState);
	// ~ Indicator

	// GAS Helpers
	UPROPERTY(ReplicatedUsing = OnRep_ActiveBuffHandle)
	FActiveGameplayEffectHandle ActiveBuffHandle;

	UFUNCTION(Server, Reliable)
	void Server_ApplyBuff(TSubclassOf<UGameplayEffect> BuffClass);

	UFUNCTION()
	void OnRep_ActiveBuffHandle();

	UFUNCTION(Server, Reliable)
	void Server_RemoveBuff(AActor* InActor);
	float GetGradeMultiplier() const;
	float GetComboMultiplier() const;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Instrument|Sound")
	TObjectPtr<UAkAudioEvent> BuffActivationSound;
	// ~GAS Helpers

	UPROPERTY(EditDefaultsOnly, Category = "Config|Instrument|Sound")
	TObjectPtr<UAkAudioEvent> PerfectNoteHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Instrument|Sound")
	TObjectPtr<UAkAudioEvent> GoodNoteHitSound;

	// Rhythm Logic
	UPROPERTY(EditAnywhere, Category = "Config|Instrument|Data")
	TObjectPtr<UInstrumentScoreData> ScoreData;
	UFUNCTION()
	virtual void HandleNoteDetected(ENoteResult InNoteResult);
	virtual float CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo) { return 0.f; }
	// ~Rhythm Logic

	// UI
	FTimerHandle WidgetInitTimerHandle;
	void TryCreateIndicatorWidget();

	UPROPERTY(EditDefaultsOnly, Category = "Config|Rhythm|UI")
	TSubclassOf<URhythmComboWidgetBase> ComboWidgetClass;

	UPROPERTY()
	TWeakObjectPtr<URhythmComboWidgetBase> ComboWidgetInstance = nullptr;
	// ~UI
	ADefaultPlayerState* GetOwnerPlayerState() const;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Instrument|Sound")
	TObjectPtr<UAkAudioEvent> InstrumentDropSound;

private:
	void BindToRhythmSubsystem(bool bBind);

public:
	FORCEINLINE float GetInstrumentPickUpScore() const { return ScoreData ? ScoreData->InstrumentPickUpScore : 0.0f; }
};
