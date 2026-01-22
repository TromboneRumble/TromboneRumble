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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitSuccessDelegate, AActor*, HitActor);

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
	virtual bool CanAttack() const override { return bCanAttack; };
	virtual void BeginAttack() override;
	virtual void EndAttack() override;
	// ~ End IWeapon Interface
	
	virtual void DetectHit();

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHitSuccessDelegate OnHitSuccess;

	virtual bool IsCanSweep() const;

protected:
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_OnHitSuccess(AActor* HitActor);
	UFUNCTION(Client, Reliable)
	virtual void Client_OnHitSuccess(AActor* HitActor);
	virtual void PlayHitSound();
	virtual void OnRep_CurrentOwner(AActor* OldActor) override;
	
	bool IsOwnerLocallyControlled() const;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UAkAudioEvent> HitSoundEvent;

	//악기를 들고있는 상태에서 현재 악기로 바꿀 수 있는 경우
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool CanBeSwitched = false;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> WeaponData;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	EInstrumentType InstrumentType = EInstrumentType::Background;

	UPROPERTY(EditDefaultsOnly, Category = "Config|GAS|Movement")
	TSubclassOf<UGameplayEffect> EquipMoveSpeedEffectClass;

	// 이 악기가 현재 소유자에게 걸어둔 GE 핸들
	FActiveGameplayEffectHandle EquipMoveSpeedEffectHandle;
	
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> AttackTraceChannel = ECC_GameTraceChannel1;

private:
	bool bIsDetectHit = false;
	UPROPERTY(Replicated)
	bool bCanAttack = true;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
	FTransform PreviousFrameTransform;

public:
	// ~ Begin Getters & Setters
	FORCEINLINE UPrimitiveComponent* GetCollisionComponent() const { return CapsuleComponent; }
	FORCEINLINE EInstrumentType GetInstrumentType() const { return InstrumentType; }
	FORCEINLINE TObjectPtr<UWeaponDataAsset> GetAttackData() const { return WeaponData; }
	FORCEINLINE bool IsDetectHit() const { return bIsDetectHit; }
	FORCEINLINE void SetCanAttack(const bool bNewCanAttack) { bCanAttack = bNewCanAttack; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponData ? WeaponData->WeaponType : EWeaponType::Invalid; }
	FORCEINLINE float GetAttackCooldown() const { return WeaponData->AttackCooldown; }
	// ~ End Getters & Setters
};