// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/EquipmentComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "Interfaces/Equipable.h"
#include "Interfaces/ItemEquipHandler.h"
#include "Items/ItemBase.h"
#include "Items/WeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	EquippedItems.SetNumZeroed(static_cast<int32>(EEquipmentSlotType::MAX_SLOTS));
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UEquipmentComponent, EquippedItems);
}

void UEquipmentComponent::TryEquipItem(AItemBase* ItemToEquip)
{
	if (!ItemToEquip) return;

	if (GetOwnerCharacter()->HasAuthority())
	{
		Server_EquipItem_Implementation(ItemToEquip);
	}
	else
	{
		Server_EquipItem(ItemToEquip);
	}
}

void UEquipmentComponent::TryUnequipItem(const EEquipmentSlotType Slot)
{
	if (GetOwnerCharacter()->HasAuthority())
	{
		Server_UnequipItem_Implementation(Slot);
	}
	else
	{
		Server_UnequipItem(Slot);
	}
}

TObjectPtr<AItemBase> UEquipmentComponent::GetItemInSlot(EEquipmentSlotType Slot) const
{
	const int32 SlotIndex = static_cast<int32>(Slot);
	if (EquippedItems.IsValidIndex(SlotIndex) && EquippedItems[SlotIndex])
	{
		return EquippedItems[SlotIndex];
	}
	return nullptr;
}

void UEquipmentComponent::Server_EquipItem_Implementation(AItemBase* ItemToEquip)
{
	if (!ItemToEquip) return;
	
	// TODO: ItemToEquip의 슬롯 타입을 가져오는 로직 필요
	EEquipmentSlotType Slot = EEquipmentSlotType::Weapon;
	const TObjectPtr<ACharacter> OwnerChar = GetOwnerCharacter();
    
	const int32 SlotIndex = static_cast<int32>(Slot);
	if (!EquippedItems.IsValidIndex(SlotIndex)) return;

	AItemBase* OldItem = EquippedItems[SlotIndex];
	if (OldItem)
	{
		if (IEquipable* EquipableOldItem = Cast<IEquipable>(OldItem))
		{
			EquipableOldItem->Unequip(OwnerChar);
		}
	}

	EquippedItems[SlotIndex] = ItemToEquip;
	if (IEquipable* EquipableNewItem = Cast<IEquipable>(ItemToEquip))
	{
		EquipableNewItem->Equip(OwnerChar);

		if (ADefaultPlayerState* PS = OwnerChar->GetPlayerState<ADefaultPlayerState>())
		{
			PS->EquippedWeaponClass = ItemToEquip->GetClass();
		}

		if (IItemEquipHandler* EquipHandler = GetGameModeItemEquipHandler())
		{
			EquipHandler->HandleItemEquipped(OwnerChar, ItemToEquip);
		}
	}

	BroadcastEquipmentChange(Slot, ItemToEquip, OldItem);
}

void UEquipmentComponent::Server_UnequipItem_Implementation(EEquipmentSlotType SlotToUnequip)
{
	const TObjectPtr<ACharacter> OwnerChar = GetOwnerCharacter();
	const int32 SlotIndex = static_cast<int32>(SlotToUnequip);
	if (!EquippedItems.IsValidIndex(SlotIndex)) return;
    
	AItemBase* OldItem = EquippedItems[SlotIndex];
	if (OldItem)
	{
		if (IEquipable* EquipableOldItem = Cast<IEquipable>(OldItem))
		{
			EquipableOldItem->Unequip(OwnerChar);

			if (ADefaultPlayerState* PS = OwnerChar->GetPlayerState<ADefaultPlayerState>())
			{
				if (PS->EquippedWeaponClass == OldItem->GetClass())
				{
					PS->EquippedWeaponClass = nullptr;
				}
			}

			if (IItemEquipHandler* EquipHandler = GetGameModeItemEquipHandler())
			{
				EquipHandler->HandleItemUnequipped(OwnerChar, OldItem);
			}
		}
		EquippedItems[SlotIndex] = nullptr;

		BroadcastEquipmentChange(SlotToUnequip, nullptr, OldItem);
	}
}

void UEquipmentComponent::OnRep_EquippedItems(const TArray<AItemBase*>& OldEquippedItems)
{
	for (int32 i = 0; i < EquippedItems.Num(); ++i)
	{
		AItemBase* CurrentItem = EquippedItems[i];
		AItemBase* OldItem = OldEquippedItems.IsValidIndex(i) ? OldEquippedItems[i] : nullptr;

		if (CurrentItem != OldItem)
		{
			BroadcastEquipmentChange(static_cast<EEquipmentSlotType>(i), CurrentItem, OldItem);
		}
	}
}

void UEquipmentComponent::BroadcastEquipmentChange(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem) const
{
	OnEquipmentChangedDelegate.Broadcast(Slot, NewItem, OldItem);
}

IItemEquipHandler* UEquipmentComponent::GetGameModeItemEquipHandler() const
{
	AGameModeBase* GM = GetWorld()->GetAuthGameMode();
	return Cast<IItemEquipHandler>(GM);
}

TObjectPtr<ACharacter> UEquipmentComponent::GetOwnerCharacter()
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwner());
	}
	return OwnerCharacter;
}
