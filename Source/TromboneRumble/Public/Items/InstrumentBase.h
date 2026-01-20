// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/WeaponBase.h"
#include "Utilities/Defines.h" 
#include "GameplayEffectTypes.h"
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
	virtual void OnRep_Equipped() override;

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
	// ~ Indicator
	// GAS Helpers
	// 현재 활성화된 버프 핸들 (UnEquip시 제거용)
	FActiveGameplayEffectHandle ActiveBuffHandle;

	void ApplyBuff(TSubclassOf<UGameplayEffect> BuffClass);
	void RemoveBuff();
	float GetGradeMultiplier() const;
	float GetComboMultiplier() const;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Instrument|Sound")
	TObjectPtr<UAkAudioEvent> BuffActivationSound;
	// ~GAS Helpers

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

	bool IsOwnerLocallyControlled() const;
	ADefaultPlayerState* GetOwnerPlayerState() const;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Instrument|Sound")
	TObjectPtr<UAkAudioEvent> InstrumentDropSound;

private:
	void BindToRhythmSubsystem(bool bBind);
};
