// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Interfaces/Equipable.h"
#include "Items/ItemBase.h"
#include "InstrumentBase.generated.h"

class UAttackDataAsset;
class IInstrumentEventHandler;
class UGameplayEffect;

UCLASS(Abstract)
class TROMBONERUMBLE_API AInstrumentBase : public AItemBase, public IEquipable
{
	GENERATED_BODY()

public:
	AInstrumentBase();

	// Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override; 
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual void Equip_Implementation(AActor* OwnerActor) override;
	virtual void Unequip_Implementation(AActor* OwnerActor) override;
	// ~Interfaces

	FORCEINLINE TObjectPtr<UAttackDataAsset> GetAttackData() const { return AttackData; }

	void AttachToIdleSocket() const;
	void AttachToAttackSocket() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_Equipped();
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Equipped)
	uint8 bIsEquipped : 1 = 0;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttackDataAsset> AttackData;

	UPROPERTY(EditAnywhere)
	EInstrumentType InstrumentType = EInstrumentType::Invalid;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Movement")
	TSubclassOf<UGameplayEffect> EquipMoveSpeedEffectClass;

	// 이 악기가 현재 소유자에게 걸어둔 GE 핸들
	FActiveGameplayEffectHandle EquipMoveSpeedEffectHandle;

private:
	FTransform OriginMeshTransform;
	
	UPROPERTY(EditAnywhere, Category="Config")
	float ForwardImpulse = 500.0f;

	UPROPERTY(EditAnywhere, Category="Config")
	float UpwardImpulse = 300.0f;
	
	FName AttachSocketNameTromboneIdle = TEXT("socket_hand_r_idle");
	FName AttachSocketNameTromboneAttack = TEXT("socket_hand_l_attack");

public:
	//getter setter
	FORCEINLINE EInstrumentType GetInstrumentType() const { return InstrumentType; }
};
