// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Item_Trumpet.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "GameFramework/Character.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/InstrumentEventHandler.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AItem_Trumpet::AItem_Trumpet()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    TrumpetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TrumpetMesh"));
    SetRootComponent(TrumpetMesh);
    TrumpetMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
    TrumpetMesh->SetSimulatePhysics(true);

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

    Execute_Equip(this, InstigatorActor);
}

void AItem_Trumpet::Equip_Implementation(AActor* OwnerActor)
{
    if (!HasAuthority() || bIsEquipped || !OwnerActor) return;
    
    SetOwner(OwnerActor);
    CurrentOwner = OwnerActor;
    bIsEquipped = true;

    OnRep_Equipped();

    if (InteractTrigger) InteractTrigger->SetTriggerActive(false);
    
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

void AItem_Trumpet::Unequip_Implementation(AActor* OwnerActor)
{
    if (!HasAuthority() || !bIsEquipped) return;

    const FVector vForwardImpulse = CurrentOwner->GetActorForwardVector() * ForwardImpulse;
    const FVector vUpwardImpulse = FVector::UpVector * UpwardImpulse;
    
    CurrentOwner = nullptr;
    bIsEquipped = false;

    OnRep_Equipped();
    
    if (InteractTrigger) InteractTrigger->SetTriggerActive(true);
    if (TrumpetMesh) TrumpetMesh->AddImpulse(vForwardImpulse + vUpwardImpulse);

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
        if (CurrentOwner)
        {
            if (ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner))
            {
                TrumpetMesh->AttachToComponent(
                    OwnerChar->GetMesh(),
                    FAttachmentTransformRules::SnapToTargetIncludingScale,
                    AttachSocketName);
            }
            else
            {
                AttachToActor(CurrentOwner, FAttachmentTransformRules::KeepWorldTransform);
            }
        }
        SetPhysicsEnabled(false);
        PlaySound();
    }
    else
    {
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        SetPhysicsEnabled(true);
        StopSound();
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

IInstrumentEventHandler* AItem_Trumpet::GetInstrumentEventHandler() const
{
    AGameModeBase* const CurrentGameMode = GetWorld()->GetAuthGameMode();
    if (CurrentGameMode && CurrentGameMode->Implements<UInstrumentEventHandler>())
    {
        return Cast<IInstrumentEventHandler>(CurrentGameMode);
    }
    return nullptr;
}