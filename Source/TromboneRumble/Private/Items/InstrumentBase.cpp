// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/InstrumentBase.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/InstrumentEventHandler.h"
#include "Net/UnrealNetwork.h"

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

	if (ADefaultTromboneCharacter* OwnerCharacter = Cast<ADefaultTromboneCharacter>(OwnerActor))
	{
		OwnerCharacter->OnRagdollDelegate.AddDynamic(this, &AInstrumentBase::HandleUnequip);
	}

	OnRep_Equipped();

	if (InteractTriggerComponent) InteractTriggerComponent->SetTriggerActive(false);
    
	if (IInstrumentEventHandler* EventHandler = GetInstrumentEventHandler())
	{
		if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			if (APlayerController* PlayerController = OwnerPawn->GetController<APlayerController>())
			{
				EventHandler->NotifyInstrumentEquipped(PlayerController, this);
			}
		}
	}
}

void AInstrumentBase::Unequip_Implementation(AActor* OwnerActor)
{
	if (!HasAuthority() || !bIsEquipped) return;

	const FVector VForwardImpulse = CurrentOwner->GetActorForwardVector() * ForwardImpulse;
	const FVector VUpwardImpulse = FVector::UpVector * UpwardImpulse;
    
	CurrentOwner = nullptr;
	bIsEquipped = false;

	if (ADefaultTromboneCharacter* OwnerCharacter = Cast<ADefaultTromboneCharacter>(OwnerActor))
	{
		OwnerCharacter->OnRagdollDelegate.RemoveDynamic(this, &AInstrumentBase::HandleUnequip);
	}

	OnRep_Equipped();
    
	if (InteractTriggerComponent) InteractTriggerComponent->SetTriggerActive(true);
	if (ItemMeshComponent) ItemMeshComponent->AddImpulse(VForwardImpulse + VUpwardImpulse);

	if (IInstrumentEventHandler* EventHandler = GetInstrumentEventHandler())
	{
		if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			if (APlayerController* PlayerController = OwnerPawn->GetController<APlayerController>())
			{
				EventHandler->NotifyInstrumentUnequipped(PlayerController, this);
			}
		}
	}
}

void AInstrumentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInstrumentBase, bIsEquipped);
}

void AInstrumentBase::PlaySound() const
{
	if (AudioComponent) AudioComponent->Play();
}

void AInstrumentBase::StopSound() const
{
	if (AudioComponent) AudioComponent->Stop();
}

IInstrumentEventHandler* AInstrumentBase::GetInstrumentEventHandler() const
{
	AGameModeBase* const CurrentGameMode = GetWorld()->GetAuthGameMode();
	if (CurrentGameMode && CurrentGameMode->Implements<UInstrumentEventHandler>())
	{
		return Cast<IInstrumentEventHandler>(CurrentGameMode);
	}
	return nullptr;
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
					AttachSocketName);
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
	}
}

void AInstrumentBase::HandleUnequip()
{
	Execute_Unequip(this, CurrentOwner);
}