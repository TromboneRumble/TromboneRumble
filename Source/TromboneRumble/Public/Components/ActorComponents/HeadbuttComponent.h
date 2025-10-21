// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackComponent.h"
#include "Components/ActorComponent.h"
#include "HeadbuttComponent.generated.h"

UCLASS()
class TROMBONERUMBLE_API UHeadbuttComponent : public UAttackComponent
{
	GENERATED_BODY()

public:	
	virtual void Attack() override;

protected:
	UFUNCTION(Server, Reliable)
	void Server_ExecuteAttack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackEffects(const FVector& TraceStart, const FVector& TraceEnd, const bool bHit);

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> HeadbuttAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Headbutt")
	float AttackRadius = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Headbutt")
	float AttackDistance = 100.0f;
};