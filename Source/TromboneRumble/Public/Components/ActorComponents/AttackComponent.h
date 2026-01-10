// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

enum class EWeaponType : uint8;
class AWeaponBase;
enum class EEquipmentSlotType : uint8;
class AItemBase;
class UCapsuleComponent;
class UCharacterAnimInstance;
class UWeaponDataAsset;

UCLASS()
class TROMBONERUMBLE_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttackComponent();
	virtual void BeginPlay() override;
	virtual void Attack();

protected:
	UFUNCTION(Server, Reliable)
	void Server_ExecuteAttack();

	UFUNCTION(Server, Reliable)
	void Server_ExecuteAttackEnd();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackEffects();
	
	UFUNCTION(Client, Reliable)
	void Client_OnAttackRejected();
	
	void PlayAttackEffects() const;
	void UpdateAttackDelegateBinding(const bool bIsAttack);
	
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem);

private:
	UPROPERTY(EditDefaultsOnly, Category = "AttackComponent")
	FName HeadSocketName = FName("head");
	
	UPROPERTY(EditDefaultsOnly, Category = "AttackComponent")
	TEnumAsByte<ECollisionChannel> AttackTraceChannel = ECC_GameTraceChannel1;
	
	UPROPERTY(EditDefaultsOnly, Category = "AttackComponent")
	TMap<EWeaponType, TObjectPtr<UAnimMontage>> AttackMontageMap;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCharacterAnimInstance> CharacterAnimInstance = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AWeaponBase> CurrentWeapon = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<AWeaponBase> DefaultWeaponInstance = nullptr;
	
public:
	// ~ Begin Getters / Setters
	FORCEINLINE void SetDefaultWeaponInstance(AWeaponBase* DefaultWeaponInst) { DefaultWeaponInstance = DefaultWeaponInst; }
	FORCEINLINE TObjectPtr<AWeaponBase> GetCurrentWeapon() const { return CurrentWeapon; }
	// ~ End Getters / Setters
};