// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/LobbyKiosk.h"

#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Framework/LobbyGameMode.h"
#include "Utilities/Defines.h"

ALobbyKiosk::ALobbyKiosk()
{
	PrimaryActorTick.bCanEverTick = false;
	KioskMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	KioskMeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	KioskMeshComp->SetSimulatePhysics(false);
	SetRootComponent(KioskMeshComp);
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComponent->SetupAttachment(RootComponent);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CapsuleComponent->SetSimulatePhysics(false);

	InteractTrigger = CreateDefaultSubobject<UInteractionTriggerComponent>(TEXT("InteractTrigger"));
}

bool ALobbyKiosk::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return !bIsUsed;
}

void ALobbyKiosk::Interact_Implementation(AActor* InstigatorActor)
{
	IInteractable::Interact_Implementation(InstigatorActor);

	Server_RequestTravel();
	bIsUsed = true;
}

void ALobbyKiosk::BeginPlay()
{
	Super::BeginPlay();

	KioskMeshComp->SetStaticMesh(KioskMesh);
}

void ALobbyKiosk::Server_RequestTravel_Implementation()
{
	if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LobbyGameMode->RequestServerTravel(EGameState::InGame);
	}
}
