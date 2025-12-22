// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/WeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Data/WeaponDataAsset.h"
#include "Interfaces/CombatReceiver.h"
#include "Subsystems/GameStateSubsystem.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	if (SkeletalMeshComponent)
	{
		SkeletalMeshComponent->SetReceivesDecals(false);
	}
	if (CapsuleComponent)
	{
		CapsuleComponent->SetReceivesDecals(false);
	}
	
	SkeletalMeshComponent->SetCollisionObjectType(AttackTraceChannel); // Object Channel 1 : Weapon
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CapsuleComponent->SetCollisionObjectType(AttackTraceChannel); // Object Channel 1 : Weapon
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void AWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsAttacking && HasAuthority())
	{
		DetectHit();
	}
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsEquipped);
	DOREPLIFETIME(ThisClass, bIsAttacking);
}

bool AWeaponBase::CanInteract_Implementation(AActor* InstigatorActor) const
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
			if (EquipComp->GetItemInSlot(EEquipmentSlotType::Weapon))
			{
				return false;
			}
		}
		return Super::CanInteract_Implementation(InstigatorActor) && !bIsEquipped;
	}
}

void AWeaponBase::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	Equip(InstigatorActor);
}

void AWeaponBase::Equip(AActor* OwnerActor)
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

void AWeaponBase::Unequip(AActor* OwnerActor)
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

	const FVector VForwardImpulse = CurrentOwner->GetActorForwardVector() * WeaponData->WeaponDropForwardImpulse;
	const FVector VUpwardImpulse = FVector::UpVector * WeaponData->WeaponDropUpwardImpulse;

	CurrentOwner = nullptr;
	bIsEquipped = false;

	OnRep_Equipped();
    
	if (InteractTriggerComponent) InteractTriggerComponent->SetTriggerActive(true);
	if (SkeletalMeshComponent) SkeletalMeshComponent->AddImpulse(VForwardImpulse + VUpwardImpulse);
}

void AWeaponBase::DetectHit()
{
	if (!CurrentOwner || !CurrentOwner->HasAuthority() || !IsCanSweep()) return;
	
	UPrimitiveComponent* CollisionComp = GetCollisionComponent();
	if (!CollisionComp) return;

	const FTransform CurrentTransform = CollisionComp->GetComponentTransform();
	const FVector Start = PreviousFrameTransform.GetLocation();
	const FVector End = CurrentTransform.GetLocation();
	const FRotator Rotation = CurrentTransform.GetRotation().Rotator();

	FComponentQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(CurrentOwner);

	TArray<FHitResult> HitResults;

	const bool bHit = GetWorld()->SweepMultiByChannel(
	   HitResults,
	   Start,
	   End,
	   Rotation.Quaternion(),
	   AttackTraceChannel,
	   CollisionComp->GetCollisionShape(),
	   Params
	);
	
	if (!bHit) return;

    for (const FHitResult& Hit : HitResults)
    {
    	AActor* HitActor = Hit.GetActor();
    	if (HitActor && !AlreadyHitActors.Contains(HitActor) && HitActor != CurrentOwner)
    	{
    		if (ICombatReceiver* CombatReceiver = Cast<ICombatReceiver>(HitActor))
    		{
    			AlreadyHitActors.Add(HitActor);

    			ACharacter* OwnerCharacter = Cast<ACharacter>(CurrentOwner);
    			if (!OwnerCharacter) continue;
    			
    			FHitData HitData;
    			HitData.Initiator = OwnerCharacter;
    			HitData.HitDirection = (Hit.ImpactPoint - CurrentOwner->GetActorLocation()).GetSafeNormal();
    			HitData.HitType = WeaponData->HitType;

    			CombatReceiver->OnHitReceived(HitData);
    		}
    		
    		if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
    		{
    			FVector KnockbackDir = (HitActor->GetActorLocation() - CurrentOwner->GetActorLocation()).GetSafeNormal();
    			KnockbackDir.Z = 0.5f;
    			
    			HitCharacter->LaunchCharacter(KnockbackDir * WeaponData->KnockbackForce, true, true);
    		}
    	}
    }

	PreviousFrameTransform = CurrentTransform;
}

bool AWeaponBase::IsCanSweep() const
{
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return false;

	UGameStateSubsystem* GameStateSubsystem = GI->GetSubsystem<UGameStateSubsystem>();
	if (!GameStateSubsystem || GameStateSubsystem->GetGameState() != EGameState::InGame)
	{
		return false;
	}
	
	return true;
}

bool AWeaponBase::IsCanAttack() const
{
	return bCanAttack;
}

void AWeaponBase::BeginAttack()
{
	bIsAttacking = true;
	bCanAttack = false;
	AlreadyHitActors.Empty();
    
	if (const UPrimitiveComponent* CollisionComp = GetCollisionComponent())
	{
		PreviousFrameTransform = CollisionComp->GetComponentTransform();
	}
    
	SetActorTickEnabled(true); 
}

void AWeaponBase::EndAttack()
{
	bIsAttacking = false;
	bCanAttack = true;
	AlreadyHitActors.Empty();
	SetActorTickEnabled(false); 
}

void AWeaponBase::OnRep_Equipped()
{
	if (bIsEquipped)
	{
		if (!CurrentOwner) return;

		const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner);
		if (!OwnerChar) return;
		
		SetPhysicsEnabled(false);
		AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, EquipSocketName);
		SkeletalMeshComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		SkeletalMeshComponent->IgnoreActorWhenMoving(CurrentOwner, true);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	else
	{
		EndAttack();
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetPhysicsEnabled(true);
		SkeletalMeshComponent->IgnoreActorWhenMoving(CurrentOwner, false);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}