// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Interfaces/Equipable.h"
#include "Items/ItemBase.h"
#include "Subsystems/RhythmSubsystem.h"
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

	// ~ Begin IInteractable Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override; 
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	// ~ End IInteractable Interfaces
	
	// ~ Begin IEquipable Interfaces
	virtual void Equip_Implementation(AActor* OwnerActor) override;
	virtual void Unequip_Implementation(AActor* OwnerActor) override;
	// ~ End IEquipable Interfaces

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_Equipped();
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Equipped)
	uint8 bIsEquipped : 1 = 0;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttackDataAsset> AttackData;

	UPROPERTY(EditDefaultsOnly)
	EInstrumentType InstrumentType = EInstrumentType::Invalid;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Movement")
	TSubclassOf<UGameplayEffect> EquipMoveSpeedEffectClass;

	// 이 악기가 현재 소유자에게 걸어둔 GE 핸들
	FActiveGameplayEffectHandle EquipMoveSpeedEffectHandle;

private:
	UPROPERTY(EditAnywhere, Category = "Instrument|Config")
	float ForwardImpulse = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Instrument|Config")
	float UpwardImpulse = 300.0f;
	
	FName TromboneSocketName = TEXT("socket_hand_l");

public:
	// ~ Begin Getters & Setters
	FORCEINLINE EInstrumentType GetInstrumentType() const { return InstrumentType; }
	FORCEINLINE TObjectPtr<UAttackDataAsset> GetAttackData() const { return AttackData; }
	// ~ End Getters & Setters
};