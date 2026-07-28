// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/AttackComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Data/WeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "Items/WeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Utilities/DebugHelper.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAttackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bAttackInProgress, COND_OwnerOnly);
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
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
	if (!CurrentWeapon)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("No weapon equipped. Cannot perform attack."));
		return;
	}
	
	if (bAttackInProgress || IsLocalAttackPredicted())
	{
		return;
	}
	
	if (OwnerCharacter->IsLocallyControlled())
	{
		PlayAttackEffects();
		LocalAttackPredictedUntilSeconds = GetWorld()->GetTimeSeconds() + GetAttackMontagePlayTime(CurrentWeapon->GetWeaponType());
	}
	
	Server_ExecuteAttack();
}

void UAttackComponent::Server_ExecuteAttack_Implementation()
{
	if (!CurrentWeapon)
	{
		return;
	}

	if (bAttackInProgress)
	{
		Client_OnAttackRejected();
		return;
	}

	bAttackInProgress = true;
	UpdateAttackDelegateBinding(true);
	Multicast_PlayAttackEffects();

	const float MontagePlayTime = GetAttackMontagePlayTime(CurrentWeapon->GetWeaponType());
	const float FailsafeSeconds = (MontagePlayTime > 0.f) ? MontagePlayTime : 3.f;
	constexpr float FailsafeMargin = 0.5f;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ServerAttackFailsafe, this,
		&ThisClass::HandleServerAttackFailsafe, FailsafeSeconds + FailsafeMargin, false);
}

void UAttackComponent::HandleServerAttackFailsafe()
{
	if (!bAttackInProgress)
	{
		return;
	}

	LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Attack failsafe triggered: montage end event was never received. Forcing attack end."));
	Server_ExecuteAttackEnd_Implementation();
}

void UAttackComponent::Server_ExecuteAttackEnd_Implementation()
{
	bAttackInProgress = false;
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_ServerAttackFailsafe);
	}

	if (!CurrentWeapon)
	{
		return;
	}
	
	CurrentWeapon->EndAttack();
	UpdateAttackDelegateBinding(false);
}

void UAttackComponent::Multicast_PlayAttackEffects_Implementation()
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	PlayAttackEffects();
}

void UAttackComponent::Client_OnAttackRejected_Implementation()
{
	LocalAttackPredictedUntilSeconds = 0.f;

	if (!CurrentWeapon)
	{
		return;
	}

	if (OwnerCharacter && CharacterAnimInstance)
	{
		OwnerCharacter->StopAnimMontage();
		CharacterAnimInstance->SetIsAttacking(false);
	}

	CurrentWeapon->EndAttack();
}

void UAttackComponent::PlayAttackEffects() const
{
	if (!CurrentWeapon)
	{
		return;
	}
	
	const EWeaponType Type = CurrentWeapon->GetWeaponType();
	if (UAnimMontage* MontageToPlay = AttackMontageMap.FindRef(Type))
	{
		CharacterAnimInstance->SetIsAttacking(true);
		
		if (!OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_IsPlaying(MontageToPlay))
		{
			OwnerCharacter->PlayAnimMontage(MontageToPlay);
		}
	}
}

bool UAttackComponent::IsLocalAttackPredicted() const
{
	return GetWorld()->GetTimeSeconds() < LocalAttackPredictedUntilSeconds;
}

float UAttackComponent::GetAttackMontagePlayTime(const EWeaponType WeaponType) const
{
	if (const UAnimMontage* Montage = AttackMontageMap.FindRef(WeaponType))
	{
		return Montage->GetPlayLength() / FMath::Max(Montage->RateScale, UE_KINDA_SMALL_NUMBER);
	}
	return 0.f;
}

bool UAttackComponent::IsAttackMontage(const UAnimMontage* Montage) const
{
	if (!Montage)
	{
		return false;
	}

	for (const TPair<EWeaponType, TObjectPtr<UAnimMontage>>& Pair : AttackMontageMap)
	{
		if (Pair.Value == Montage)
		{
			return true;
		}
	}
	return false;
}

void UAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!IsAttackMontage(Montage))
	{
		return;
	}

	if (OwnerCharacter)
	{
		Server_ExecuteAttackEnd();
	}
}

void UAttackComponent::HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (Slot != EEquipmentSlotType::Weapon)
	{
		return;
	}
	
	OwnerCharacter->StopAnimMontage();
	CharacterAnimInstance->SetIsAttacking(false);
	LocalAttackPredictedUntilSeconds = 0.f;

	if (AWeaponBase* OldWeapon = Cast<AWeaponBase>(OldItem))
	{
		OldWeapon->EndAttack();
	}
	
	Server_ExecuteAttackEnd();

	if (NewItem)
	{
		if (AWeaponBase* NewInstrument = Cast<AWeaponBase>(NewItem))
		{
			CurrentWeapon = NewInstrument;
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
	if (!CharacterAnimInstance)
	{
		return;
	}

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