// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/WeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AkComponent.h"
#include "GameplayEffect.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Data/WeaponDataAsset.h"
#include "Framework/DefaultPlayerState.h"
#include "Interfaces/CombatReceiver.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/DebugHelper.h"

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
		CapsuleComponent->CanCharacterStepUpOn = ECB_No;
		//충돌판정은 Sweep을 통해서 하고, CapsuleComponent는 Collision Shape 지정용
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsDetectHit && HasAuthority())
	{
		DetectHit();
	}
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bCanAttack);
}

bool AWeaponBase::CanInteract_Implementation(AActor* InstigatorActor) const
{
	if (CanBeSwitched)
	{
		return Super::CanInteract_Implementation(InstigatorActor) && !CurrentOwner;
	}
	
	if (const UEquipmentComponent* EquipComp = InstigatorActor->FindComponentByClass<UEquipmentComponent>())
	{
		if (const TObjectPtr<AItemBase> Item = EquipComp->GetItemInSlot(EEquipmentSlotType::Weapon))
		{
			if (const AWeaponBase* Weapon = Cast<AWeaponBase>(Item))
			{
				if (Weapon->GetWeaponType() == EWeaponType::Cymbals ||
					Weapon->GetWeaponType() == EWeaponType::Violin ||
					Weapon->GetWeaponType() == EWeaponType::Trombone)
				{
					return false;
				}
			}
		}
	}
	
	return Super::CanInteract_Implementation(InstigatorActor) && !CurrentOwner;
}

void AWeaponBase::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	//Equip은 EquipComponent를 통해서만
	//Equip(InstigatorActor);
}

void AWeaponBase::Equip(AActor* OwnerActor)
{
	if (!HasAuthority() || CurrentOwner || !OwnerActor) return;
    
	SetOwner(OwnerActor);
	AActor* CachedActor = CurrentOwner;
	CurrentOwner = OwnerActor;
	
	OnRep_CurrentOwner(CachedActor);

	if (InteractTriggerComponent)
	{
		InteractTriggerComponent->SetTriggerActive(false);
		ForceNetUpdate();
	}

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
	if (!HasAuthority() || !CurrentOwner) return;

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
	AActor* CachedActor = CurrentOwner;
	CurrentOwner = nullptr;
	OnRep_CurrentOwner(CachedActor);
	

	if (InteractTriggerComponent) InteractTriggerComponent->SetTriggerActive(true);
	if (SkeletalMeshComponent) SkeletalMeshComponent->AddImpulse(VForwardImpulse + VUpwardImpulse);
}

void AWeaponBase::DetectHit()
{
	if (!CurrentOwner || !HasAuthority() || !IsCanSweep()) return;
	
	UPrimitiveComponent* CollisionComp = CapsuleComponent;
	if (!CollisionComp) return;

	const FTransform CurrentTransform = CollisionComp->GetComponentTransform();
	const FVector Start = PreviousFrameTransform.GetLocation();
	const FVector End = CurrentTransform.GetLocation();
	const FRotator Rotation = CurrentTransform.GetRotation().Rotator();

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FComponentQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(CurrentOwner);

	TArray<FHitResult> HitResults;

	const bool bHit = GetWorld()->SweepMultiByObjectType(
	   HitResults,
	   Start,
	   End,
	   Rotation.Quaternion(),
	   ObjectParams,
	   CollisionComp->GetCollisionShape(),
	   Params
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !AlreadyHitActors.Contains(HitActor) && HitActor != CurrentOwner)
			{
				AlreadyHitActors.Add(HitActor);
				Multicast_PlayHitSound();
				if (HitActor->Implements<UCombatReceiver>())
				{
					FHitData HitData;
					FVector Direction = (Hit.ImpactPoint - CurrentOwner->GetActorLocation()).GetSafeNormal();
					Direction.Z = 0.5f;
					HitData.HitDirection = Direction.GetSafeNormal();
					HitData.KnockbackForce = WeaponData->KnockbackForce;
					HitData.HitReaction = WeaponData->HitReactionType;
					HitData.HitInstigator = HitInstigatorType;
					
					Multicast_OnHitSuccess(HitActor);
					Client_OnHitSuccess(HitActor);
					ICombatReceiver::Execute_OnHitReceived(HitActor, HitData);
				}
			}
		}
	}
	PreviousFrameTransform = CurrentTransform;
}

void AWeaponBase::Multicast_OnHitSuccess_Implementation(AActor* HitActor)
{
}

void AWeaponBase::Client_OnHitSuccess_Implementation(AActor* HitActor)
{	
	OnHitSuccess.Broadcast(HitActor);
}

bool AWeaponBase::IsCanSweep() const
{
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return false;

	UGameStateSubsystem* GameStateSubsystem = GI->GetSubsystem<UGameStateSubsystem>();
	if (!GameStateSubsystem)
	{
		return false;
	}
	if (GameStateSubsystem->GetLevelState() != ELevelState::InGame && GameStateSubsystem->GetLevelState() != ELevelState::Tutorial)
	{
		return false;
	}
	
	return true;
}


void AWeaponBase::BeginAttack()
{
	bIsDetectHit = true;
	AlreadyHitActors.Empty();
	SetActorTickEnabled(true); 
	
	if (const UPrimitiveComponent* CollisionComp = GetCollisionComponent())
	{
		PreviousFrameTransform = CollisionComp->GetComponentTransform();
	}
}

void AWeaponBase::EndAttack()
{
	bIsDetectHit = false;
	AlreadyHitActors.Empty();
	SetActorTickEnabled(false); 
}

void AWeaponBase::Multicast_PlayHitSound_Implementation()
{
	if (HitSoundEvent && AkSoundComponent)
	{
		AkSoundComponent->PostAkEvent(HitSoundEvent);
	}
}

void AWeaponBase::OnRep_CurrentOwner(AActor* OldActor)
{
	Super::OnRep_CurrentOwner(OldActor);
	if (CurrentOwner)
	{
		const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner);
		if (!OwnerChar) return;

		SetPhysicsEnabled(false);
		AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponData->EquipSocketName);
		SkeletalMeshComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		SkeletalMeshComponent->IgnoreActorWhenMoving(CurrentOwner, true);
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		EndAttack();
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		if (HasAuthority())
		{
			SetPhysicsEnabled(true);
		}
		SkeletalMeshComponent->IgnoreActorWhenMoving(CurrentOwner, false);
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}
bool AWeaponBase::IsOwnerLocallyControlled() const
{
	const APawn* PawnOwner = Cast<APawn>(CurrentOwner);
	return PawnOwner && PawnOwner->IsLocallyControlled();
}
