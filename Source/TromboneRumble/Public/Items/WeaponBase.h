// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/CapsuleComponent.h"
#include "Data/WeaponDataAsset.h"
#include "Interfaces/Equipable.h"
#include "Interfaces/Weapon.h"
#include "Items/ItemBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "WeaponBase.generated.h"

class UWeaponDataAsset;
class UGameplayEffect;

UCLASS()
class TROMBONERUMBLE_API AWeaponBase : public AItemBase, public IEquipable, public IWeapon
{
	GENERATED_BODY()

public:
	AWeaponBase();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// ~ Begin IInteractable Interface
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override; 
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	// ~ End IInteractable Interfaces
	
	// ~ Begin IEquipable Interfaces
	virtual void Equip(AActor* OwnerActor) override;
	virtual void Unequip(AActor* OwnerActor) override;
	// ~ End IEquipable Interfaces
	
	// ~ Begin IWeapon Interface
	virtual bool IsCanAttack() const override;
	virtual void BeginAttack() override;
	virtual void EndAttack() override;
	// ~ End IWeapon Interface
	
	virtual void DetectHit();
	virtual bool IsCanSweep() const;

protected:
	UFUNCTION()
	virtual void OnRep_Equipped();

	//이 악기가 누군가에게 장착된 상태인지 여부
	UPROPERTY(VisibleAnywhere, Category = "Weapon", ReplicatedUsing = OnRep_Equipped)
	uint8 bIsEquipped : 1 = 0;

	//악기를 들고있는 상태에서 현재 악기로 바꿀 수 있는 경우
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool CanBeSwitched = false;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> WeaponData;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EInstrumentType InstrumentType = EInstrumentType::Background;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|GAS|Movement")
	TSubclassOf<UGameplayEffect> EquipMoveSpeedEffectClass;

	// 이 악기가 현재 소유자에게 걸어둔 GE 핸들
	FActiveGameplayEffectHandle EquipMoveSpeedEffectHandle;
	
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> AttackTraceChannel = ECC_GameTraceChannel1;

private:
	UPROPERTY(Replicated)
	bool bIsAttacking = false;
	bool bCanAttack = true;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
	FTransform PreviousFrameTransform;

public:
	// ~ Begin Getters & Setters
	FORCEINLINE UPrimitiveComponent* GetCollisionComponent() const { return CapsuleComponent; }
	FORCEINLINE EInstrumentType GetInstrumentType() const { return InstrumentType; }
	FORCEINLINE TObjectPtr<UWeaponDataAsset> GetAttackData() const { return WeaponData; }
	FORCEINLINE bool GetIsAttacking() const { return bIsAttacking; }
	FORCEINLINE void SetCanAttack(const bool bNewCanAttack) { bCanAttack = bNewCanAttack; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponData ? WeaponData->WeaponType : EWeaponType::Invalid; }
	FORCEINLINE float GetAttackCooldown() const { return WeaponData->AttackCooldown; }
	// ~ End Getters & Setters
};