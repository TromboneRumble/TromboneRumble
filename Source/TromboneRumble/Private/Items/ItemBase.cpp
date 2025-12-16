// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/ItemBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Net/UnrealNetwork.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);
	
	ItemMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetRootComponent(ItemMeshComponent);
	SetPhysicsEnabled(true);
	
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetupAttachment(RootComponent);
	CapsuleComponent->SetSimulatePhysics(false);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	InteractTriggerComponent = CreateDefaultSubobject<UInteractionTriggerComponent>(TEXT("InteractTriggerComponent"));
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
		ItemMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
		ItemMeshComponent->SetSimulatePhysics(true);
		ItemMeshComponent->SetAllBodiesSimulatePhysics(true);
	}
	else
	{
		ItemMeshComponent->SetSimulatePhysics(false);
		ItemMeshComponent->SetAllBodiesSimulatePhysics(false);
	}
}