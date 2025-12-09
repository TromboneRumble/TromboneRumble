// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/InstrumentBase.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Utilities/DebugHelper.h"

AInstrumentBase::AInstrumentBase()
{
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;
	AudioComponent->bAllowSpatialization = true;
	AudioComponent->bOverrideAttenuation = true;

	if (!InstrumentSound)
	{
		static ConstructorHelpers::FObjectFinder<USoundBase> SoundFinder(TEXT("/Game/Sounds/Trumpet.Trumpet"));
		if (SoundFinder.Succeeded())
		{
			AudioComponent->SetSound(SoundFinder.Object);
		}
	}
	
	CapsuleComponent->SetCollisionObjectType(ECC_GameTraceChannel1); // Object Channel 1 : Weapon
}

bool AInstrumentBase::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return Super::CanInteract_Implementation(InstigatorActor) && !bIsEquipped;
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
	if (ItemMeshComponent) ItemMeshComponent->AddImpulse(VForwardImpulse + VUpwardImpulse);
}

void AInstrumentBase::AttachToIdleSocket() const
{
	if (const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner))
	{
		CapsuleComponent->AttachToComponent(
			OwnerChar->GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			AttachSocketNameTromboneIdle);
	}
}

void AInstrumentBase::AttachToAttackSocket() const
{
	if (const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner))
	{
		CapsuleComponent->AttachToComponent(
			OwnerChar->GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			AttachSocketNameTromboneAttack);
	}
}

void AInstrumentBase::BeginPlay()
{
	Super::BeginPlay();

	OriginMeshTransform = ItemMeshComponent->GetRelativeTransform();
}

void AInstrumentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInstrumentBase, bIsEquipped);
}

void AInstrumentBase::PlaySound() const
{
	// if (AudioComponent) AudioComponent->Play();
}

void AInstrumentBase::StopSound() const
{
	if (AudioComponent) AudioComponent->Stop();
}

void AInstrumentBase::OnRep_Equipped()
{
	if (bIsEquipped)
	{
		SetPhysicsEnabled(false);
		if (CurrentOwner)
		{
			if (const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner))
			{
				CapsuleComponent->AttachToComponent(
					OwnerChar->GetMesh(),
					FAttachmentTransformRules::SnapToTargetIncludingScale,
					AttachSocketNameTromboneIdle);
				ItemMeshComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
			}
			else
			{
				AttachToActor(CurrentOwner, FAttachmentTransformRules::KeepWorldTransform);
			}
		}
		PlaySound();
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetPhysicsEnabled(true);
		StopSound();
		ItemMeshComponent->SetRelativeTransform(OriginMeshTransform);
	}
}