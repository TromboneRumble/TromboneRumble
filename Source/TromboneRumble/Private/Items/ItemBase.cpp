// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/ItemBase.h"

#include "AkComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Net/UnrealNetwork.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMeshComponent"));
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetRootComponent(SkeletalMeshComponent);
	SetPhysicsEnabled(true);
	
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetupAttachment(RootComponent);
	CapsuleComponent->SetSimulatePhysics(false);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	InteractTriggerComponent = CreateDefaultSubobject<UInteractionTriggerComponent>(TEXT("InteractTriggerComponent"));

	AkSoundComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkSoundComponent"));
	if (AkSoundComponent)
	{
		AkSoundComponent->SetupAttachment(RootComponent);
		AkSoundComponent->OcclusionRefreshInterval = 0.f;
	}
}

void AItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemBase, CurrentOwner);
}

bool AItemBase::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return CurrentOwner == nullptr;
}

void AItemBase::SetPhysicsEnabled(bool bEnable) const
{
	if (bEnable)
	{
		SkeletalMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
		SkeletalMeshComponent->SetSimulatePhysics(true);
		SkeletalMeshComponent->SetAllBodiesSimulatePhysics(true);
	}
	else
	{
		SkeletalMeshComponent->SetSimulatePhysics(false);
		SkeletalMeshComponent->SetAllBodiesSimulatePhysics(false);
	}
}