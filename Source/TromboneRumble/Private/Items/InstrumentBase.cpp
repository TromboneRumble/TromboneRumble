// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/InstrumentBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

AInstrumentBase::AInstrumentBase()
{
	if (SkeletalMeshComponent)
	{
		SkeletalMeshComponent->SetReceivesDecals(false);
	}
	if (CapsuleComponent)
	{
		CapsuleComponent->SetReceivesDecals(false);
	}
	
	SkeletalMeshComponent->SetCollisionObjectType(ECC_GameTraceChannel1); // Object Channel 1 : Weapon
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CapsuleComponent->SetCollisionObjectType(ECC_GameTraceChannel1); // Object Channel 1 : Weapon
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

bool AInstrumentBase::CanInteract_Implementation(AActor* InstigatorActor) const
{
	//플레이어의 악기 스위칭이 가능하면 현재 악기가 누군가에게 장착됐는지 여부만 확인
	if (CanBeSwitched)
	{
		return Super::CanInteract_Implementation(InstigatorActor) && !bIsEquipped;
	}
	// 악기 스위칭이 불가하면 플레이어가 악기를 들고있는지 확인
	else
	{
		//플레이어가 악기를 들고있으면 false반환
		if (UEquipmentComponent* EquipComp = InstigatorActor->FindComponentByClass<UEquipmentComponent>())
		{
			if (EquipComp->GetItemInSlot(EEquipmentSlotType::Instrument))
			{
				return false;
			}
		}
		return Super::CanInteract_Implementation(InstigatorActor) && !bIsEquipped;
	}
}

void AInstrumentBase::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	Execute_Equip(this, InstigatorActor);
}

void AInstrumentBase::Equip_Implementation(AActor* OwnerActor)
{
	if (!HasAuthority() || bIsEquipped || !OwnerActor) return;
    
	SetOwner(OwnerActor);
	CurrentOwner = OwnerActor;
	bIsEquipped = true;

	OnRep_Equipped();

	if (InteractTriggerComponent) InteractTriggerComponent->SetTriggerActive(false);

	// Move Speed Gameplay Effect 적용
	if (EquipMoveSpeedEffectClass)
	{
		if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OwnerActor))
		{
			if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
			{
				const UGameplayEffect* GE = EquipMoveSpeedEffectClass->GetDefaultObject<UGameplayEffect>();

				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddSourceObject(this);

				EquipMoveSpeedEffectHandle =
					ASC->ApplyGameplayEffectToSelf(GE, 1.f, Context);
			}
		}
	}
}

void AInstrumentBase::Unequip_Implementation(AActor* OwnerActor)
{
	if (!HasAuthority() || !bIsEquipped) return;

	if (EquipMoveSpeedEffectHandle.IsValid())
	{
		if (OwnerActor)
		{
			if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OwnerActor))
			{
				if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
				{
					ASC->RemoveActiveGameplayEffect(EquipMoveSpeedEffectHandle);
				}
			}
		}

		EquipMoveSpeedEffectHandle.Invalidate();
	}

	const FVector VForwardImpulse = CurrentOwner->GetActorForwardVector() * ForwardImpulse;
	const FVector VUpwardImpulse = FVector::UpVector * UpwardImpulse;

	CurrentOwner = nullptr;
	bIsEquipped = false;

	OnRep_Equipped();
    
	if (InteractTriggerComponent) InteractTriggerComponent->SetTriggerActive(true);
	if (SkeletalMeshComponent) SkeletalMeshComponent->AddImpulse(VForwardImpulse + VUpwardImpulse);
}

void AInstrumentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInstrumentBase, bIsEquipped);
}

void AInstrumentBase::OnRep_Equipped()
{
	if (bIsEquipped)
	{
		if (!CurrentOwner) return;

		const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner);
		if (!OwnerChar) return;
		
		SetPhysicsEnabled(false);
		AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TromboneSocketName);
		SkeletalMeshComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		SkeletalMeshComponent->IgnoreActorWhenMoving(CurrentOwner, true);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetPhysicsEnabled(true);
		SkeletalMeshComponent->IgnoreActorWhenMoving(CurrentOwner, false);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}