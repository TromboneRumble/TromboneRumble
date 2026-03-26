// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/AttackComponent.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Data/WeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "Items/WeaponBase.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
	{
		CharacterAnimInstance = Cast<UCharacterAnimInstance>(Mesh->GetAnimInstance());
	}

	if (UEquipmentComponent* EquipmentComp = OwnerCharacter->FindComponentByClass<UEquipmentComponent>())
	{
		EquipmentComp->OnEquipmentChangedDelegate.AddDynamic(this, &UAttackComponent::HandleOnEquipmentChanged);
		
		AItemBase* CurrentEquippedWeapon = EquipmentComp->GetItemInSlot(EEquipmentSlotType::Weapon);
		HandleOnEquipmentChanged(EEquipmentSlotType::Weapon, CurrentEquippedWeapon, nullptr);
	}
}

void UAttackComponent::Attack()
{
	if (!CurrentWeapon || CurrentWeapon->IsDetectHit() || !CurrentWeapon->CanAttack()) return;
	
	if (OwnerCharacter->IsLocallyControlled())
	{
		PlayAttackEffects();
	}
	
	Server_ExecuteAttack();
}

void UAttackComponent::Server_ExecuteAttack_Implementation()
{
	if (!CurrentWeapon) return;

	if (CurrentWeapon->IsDetectHit())
	{
		Client_OnAttackRejected();
		return;
	}
	
	CurrentWeapon->SetCanAttack(false);
	UpdateAttackDelegateBinding(true);
	Multicast_PlayAttackEffects();
}

void UAttackComponent::Server_ExecuteAttackEnd_Implementation()
{
	if (!CurrentWeapon) return;
	
	CurrentWeapon->SetCanAttack(true);
	UpdateAttackDelegateBinding(false);
}

void UAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled()) return;

	PlayAttackEffects();
}

void UAttackComponent::Client_OnAttackRejected_Implementation()
{
	if (!CurrentWeapon) return;

	if (OwnerCharacter && CharacterAnimInstance)
	{
		OwnerCharacter->StopAnimMontage();
		CharacterAnimInstance->SetIsAttacking(false);
	}

	CurrentWeapon->EndAttack();
	CurrentWeapon->SetCanAttack(true);
}

void UAttackComponent::PlayAttackEffects() const
{
	if (!CurrentWeapon) return;
	
	const EWeaponType Type = CurrentWeapon->GetWeaponType();
	if (UAnimMontage* MontageToPlay = AttackMontageMap.FindRef(Type))
	{
		CurrentWeapon->SetCanAttack(false);
		CharacterAnimInstance->SetIsAttacking(true);
		
		if (!OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_IsPlaying(MontageToPlay))
		{
			OwnerCharacter->PlayAnimMontage(MontageToPlay);
		}
	}
}

void UAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		Server_ExecuteAttackEnd();
	}
}

void UAttackComponent::HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (Slot != EEquipmentSlotType::Weapon) return;
	
	OwnerCharacter->StopAnimMontage();
	CharacterAnimInstance->SetIsAttacking(false);
	
	if (AWeaponBase* OldWeapon = Cast<AWeaponBase>(OldItem))
	{
		OldWeapon->SetCanAttack(true);
		OldWeapon->EndAttack();
	}
	
	Server_ExecuteAttackEnd();

	if (NewItem)
	{
		if (AWeaponBase* NewInstrument = Cast<AWeaponBase>(NewItem))
		{
			CurrentWeapon = NewInstrument;
			CurrentWeapon->SetCanAttack(true);
			CurrentWeapon->EndAttack();
		}
	}
	else
	{
		CurrentWeapon = DefaultWeaponInstance;
	}
}

void UAttackComponent::UpdateAttackDelegateBinding(const bool bIsAttack)
{
	if (!CharacterAnimInstance) return;

	if (bIsAttack)
	{
		if (!CharacterAnimInstance->OnMontageEnded.IsAlreadyBound(this, &ThisClass::OnAttackMontageEnded))
		{
			CharacterAnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnAttackMontageEnded);
		}
	}
	else
	{
		CharacterAnimInstance->OnMontageEnded.RemoveDynamic(this, &ThisClass::OnAttackMontageEnded);
	}
}