// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InstrumentScoreData.generated.h"

class UGameplayEffect;

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UInstrumentScoreData : public UDataAsset
{
	GENERATED_BODY()
public:
	// --- Base Scores ---
	UPROPERTY(EditAnywhere, Category = "Scoring|Base")
	float PerfectScore = 100.f;

	UPROPERTY(EditAnywhere, Category = "Scoring|Base")
	float GoodScore = 50.f;

	UPROPERTY(EditAnywhere, Category = "Scoring|Combo")
	float ComboBasePoint = 10.f; 

	UPROPERTY(EditAnywhere, Category = "Scoring|Attack")
	float AttackScore = 200.f;

	// --- Thresholds & GAS ---
	UPROPERTY(EditAnywhere, Category = "Logic|Violin")
	int32 ViolinBuffActivationCount = 10; // 몇 노트마다 버프?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Violin")
	int32 ViolinBuffDurationCount = 5;

	UPROPERTY(EditAnywhere, Category = "Logic|Violin")
	TSubclassOf<UGameplayEffect> ViolinBuffEffectClass;

	UPROPERTY(EditAnywhere, Category = "Logic|Trombone")
	int32 TromboneBuffComboThreshold = 20; // 몇 콤보부터 버프?

	UPROPERTY(EditAnywhere, Category = "Logic|Trombone")
	TSubclassOf<UGameplayEffect> TromboneBuffEffectClass;
};
