// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/InstrumentBase.h"
#include "InstrumentCymbals.generated.h"

/**
 * 심벌즈는 반쪽짜리로 맵에 스폰되기 때문에 장착시 남은 반쪽을 생성후, 플레이어의 오른손에 달아줘야함.
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API AInstrumentCymbals : public AInstrumentBase
{
	GENERATED_BODY()
public:
	virtual void Multicast_OnHitSuccess_Implementation(AActor* HitActor) override;
	virtual void Client_OnHitSuccess_Implementation(AActor* HitActor) override;
protected:
	virtual void OnRep_CurrentOwner(AActor* OldActor) override;
	virtual float CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo) override;

	UPROPERTY(EditDefaultsOnly , Category = "Config")
	TSubclassOf<AActor> CymbalsHalfClass = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CymbalsRightHandActor = nullptr;
};
