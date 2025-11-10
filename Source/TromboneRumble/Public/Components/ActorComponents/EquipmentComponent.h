// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class IItemEquipHandler;
class AItemBase;
enum class EEquipmentSlotType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentChangedSignature, EEquipmentSlotType, Slot, AItemBase*, NewItem, AItemBase*, OldItem);

UCLASS()
class TROMBONERUMBLE_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEquipmentComponent();
	void TryEquipItem(AItemBase* ItemToEquip);
	void TryUnequipItem(EEquipmentSlotType Slot);
	TObjectPtr<AItemBase> GetItemInSlot(EEquipmentSlotType Slot) const;

	FORCEINLINE void InitializeOwner(ACharacter* InOwnerCharacter) { OwnerCharacter = InOwnerCharacter; }
	FOnEquipmentChangedSignature OnEquipmentChangedDelegate;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_EquipItem(AItemBase* ItemToEquip);

	UFUNCTION(Server, Reliable)
	void Server_UnequipItem(EEquipmentSlotType SlotToUnequip);

	UFUNCTION()
	void OnRep_EquippedItems(const TArray<AItemBase*>& OldEquippedItems);

	UPROPERTY(ReplicatedUsing = OnRep_EquippedItems)
	TArray<AItemBase*> EquippedItems;
	
	void BroadcastEquipmentChange(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem) const;

private:
	IItemEquipHandler* GetGameModeItemEquipHandler() const;
	
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;
};
