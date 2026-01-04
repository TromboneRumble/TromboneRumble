// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/WeaponBase.h"
#include "Utilities/Defines.h" 
#include "GameplayEffectTypes.h"
#include "InstrumentBase.generated.h"

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
	virtual void OnRep_Equipped() override;

	UPROPERTY(EditAnywhere, Category = "Instrument|Data")
	TObjectPtr<UInstrumentScoreData> ScoreData;

	// 현재 활성화된 버프 핸들 (UnEquip시 제거용)
	FActiveGameplayEffectHandle ActiveBuffHandle;

	// --- GAS Helpers ---
	void ApplyBuff(TSubclassOf<UGameplayEffect> BuffClass);
	void RemoveBuff();
	float GetGradeMultiplier() const;
	float GetComboMultiplier() const;

	// --- Rhythm Logic ---
	UFUNCTION()
	void HandleNoteDetected(ENoteResult InNoteResult);
	virtual float CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo) { return 0.f; }

	bool IsOwnerLocallyControlled() const;
	ADefaultPlayerState* GetOwnerPlayerState() const;

private:
	void BindToRhythmSubsystem(bool bBind);
};
