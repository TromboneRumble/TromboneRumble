// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Equipable.h"
#include "Items/ItemBase.h"
#include "InstrumentBase.generated.h"

class UAttackDataAsset;
class IInstrumentEventHandler;

UCLASS()
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

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void PlaySound() const;
	void StopSound() const;

	UFUNCTION()
	virtual void OnRep_Equipped();
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Equipped)
	uint8 bIsEquipped : 1 = 0;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAudioComponent> AudioComponent = nullptr;

	UPROPERTY(EditAnywhere, Category="Config")
	TObjectPtr<USoundBase> InstrumentSound = nullptr;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttackDataAsset> AttackData;

private:
	UFUNCTION()
	void HandleUnequip();
	
	UPROPERTY(EditAnywhere, Category="Config")
	float ForwardImpulse = 500.0f;

	UPROPERTY(EditAnywhere, Category="Config")
	float UpwardImpulse = 300.0f;
	
	FName AttachSocketName = TEXT("socket_hand_r");
};
