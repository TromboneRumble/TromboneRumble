// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item_Trumpet.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "GameFramework/Character.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"

AItem_Trumpet::AItem_Trumpet()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    TrumpetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TrumpetMesh"));
    SetRootComponent(TrumpetMesh);
    TrumpetMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
    TrumpetMesh->SetSimulatePhysics(false);

    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    CapsuleComponent->SetupAttachment(RootComponent);
    CapsuleComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CapsuleComponent->SetSimulatePhysics(false);

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    AudioComponent->bAllowSpatialization = true;
    AudioComponent->bOverrideAttenuation = true;

    InteractTrigger = CreateDefaultSubobject<UInteractionTriggerComponent>(TEXT("InteractTrigger"));

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> TrumpetMeshFinder(TEXT("/Game/Arts/Trumpet/SKM_Trumpet.SKM_Trumpet"));
    if (TrumpetMeshFinder.Succeeded())
    {
        TrumpetMesh->SetSkeletalMesh(TrumpetMeshFinder.Object);
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> TrumpetSoundFinder(TEXT("/Game/Sounds/Trumpet.Trumpet"));
    if (TrumpetSoundFinder.Succeeded())
    {
        AudioComponent->SetSound(TrumpetSoundFinder.Object);
    }
}

bool AItem_Trumpet::CanInteract_Implementation(AActor* InstigatorActor) const
{
    // 이미 누군가 장착 중이면 못 줍기
    return !bIsEquipped;
}

void AItem_Trumpet::Interact_Implementation(AActor* InstigatorActor)
{
    if (!HasAuthority()) return;

    IEquipable::Execute_Equip(this, InstigatorActor);
}

void AItem_Trumpet::Equip_Implementation(AActor* OwnerActor)
{
    if (!HasAuthority() || bIsEquipped) return;
    CurrentOwner = OwnerActor;
    bIsEquipped = true;
    OnRep_Equipped();

    SetPickupTriggerEnabled_Server(false);

    if (ACharacter* OwnerChar = Cast<ACharacter>(OwnerActor))
    {
        TrumpetMesh->AttachToComponent(
            OwnerChar->GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            AttachSocketName);
    }
    else
    {
        AttachToActor(OwnerActor, FAttachmentTransformRules::KeepWorldTransform);
    }
}

void AItem_Trumpet::Unequip_Implementation(AActor* OwnerActor)
{
	IEquipable::Unequip_Implementation(OwnerActor);
}

void AItem_Trumpet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItem_Trumpet, bIsEquipped);
    DOREPLIFETIME(AItem_Trumpet, CurrentOwner);
}

void AItem_Trumpet::OnRep_Equipped()
{
    if (bIsEquipped)
    {
        PlaySound();
        SetPhysicsEnabled(false);
    }
    else
    {
        StopSound();
        SetPhysicsEnabled(true);
    }
}

void AItem_Trumpet::PlaySound() const
{
    if (AudioComponent) AudioComponent->Play();
}

void AItem_Trumpet::StopSound() const
{
    if (AudioComponent) AudioComponent->Stop();
}

void AItem_Trumpet::SetPhysicsEnabled(bool bEnable) const
{
    if (!TrumpetMesh) return;
    TrumpetMesh->SetSimulatePhysics(bEnable);
    TrumpetMesh->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

    if (CapsuleComponent)
    {
        CapsuleComponent->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

void AItem_Trumpet::SetPickupTriggerEnabled_Server(bool bEnable)
{
    if (!HasAuthority() || !InteractTrigger) return;
    if (bEnable)
    {
        InteractTrigger->OnDroppedToWorld();    // 후보 재등록
    }
    else
    {
        //InteractTrigger->ActivateTrigger(false); // 후보 제거
    }
}

void AItem_Trumpet::ReEnableTriggerAfterDrop_Server()
{
    if (!HasAuthority() || !InteractTrigger) return;
    InteractTrigger->OnDroppedToWorld();
}

