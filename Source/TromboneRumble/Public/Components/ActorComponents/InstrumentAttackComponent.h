// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponents/AttackComponent.h"
#include "InstrumentAttackComponent.generated.h"

UCLASS()
class TROMBONERUMBLE_API UInstrumentAttackComponent : public UAttackComponent
{
	GENERATED_BODY()

public:
	UInstrumentAttackComponent();
	virtual void Attack() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetInstrumentMesh(const TObjectPtr<UPrimitiveComponent> InMesh) { InstrumentMesh = InMesh; }

protected:
	UFUNCTION(Server, Reliable)
	void Server_ExecuteAttack();

	UFUNCTION(Server, Reliable)
	void Server_ExecuteEndAttack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackEffects();
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> InstrumentAttackAnimMontage = nullptr;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
	
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> InstrumentMesh = nullptr;
	
	bool bIsAttacking = false;
	FTransform PreviousFrameTransform;
};
