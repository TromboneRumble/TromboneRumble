// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

class UCharacterAnimInstance;
class UAttackDataAsset;

UCLASS()
class TROMBONERUMBLE_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttackComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Attack();
	
	FORCEINLINE void SetCollisionComponent(const TObjectPtr<UPrimitiveComponent> InCollision) { CollisionComponent = InCollision; }
	FORCEINLINE void SetAttackData(UAttackDataAsset* InAttackData) { CurrentAttackData = InAttackData; }

protected:
	UFUNCTION(Server, Reliable)
	virtual void Server_ExecuteAttack();

	UFUNCTION(Server, Reliable)
	void Server_ExecuteAttackEnd();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackEffects();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteAttackEnd();
	
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY()
	TObjectPtr<UCharacterAnimInstance> CharacterAnimInstance = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttackDataAsset> CurrentAttackData = nullptr;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
	
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> CollisionComponent = nullptr;

	FTransform PreviousFrameTransform;
	bool bIsAttacking = false;
	bool bCanAttack = true;

private:
	void ResetAttackCooldown() { bCanAttack = true; }

	FTimerHandle AttackCooldownTimerHandle;
};