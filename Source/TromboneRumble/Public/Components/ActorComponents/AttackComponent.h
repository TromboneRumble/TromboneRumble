// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

class AInstrumentBase;
enum class EEquipmentSlotType : uint8;
class AItemBase;
class UCapsuleComponent;
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
	UFUNCTION()
	void HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem);

	UPROPERTY()
	TObjectPtr<UCharacterAnimInstance> CharacterAnimInstance = nullptr;
	
	UPROPERTY()
	TObjectPtr<AInstrumentBase> CurrentInstrument = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttackDataAsset> CurrentAttackData = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPrimitiveComponent> CurrentCollisionComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCapsuleComponent> HeadbuttCollisionComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttackDataAsset> HeadbuttAttackData = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	FName HeadSocketName = FName("head");
	
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> AttackTraceChannel = ECC_GameTraceChannel1;
	

	FTransform PreviousFrameTransform;
	bool bIsAttacking = false;
	bool bCanAttack = true;

private:
	bool IsCanSweep() const;
	void ResetAttackCooldown() { bCanAttack = true; }
	void SetAttackState(bool bNewState);

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
	
	FTimerHandle AttackCooldownTimerHandle;
};