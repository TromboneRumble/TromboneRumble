// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/Trumpet.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ATrumpet::ATrumpet()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	TrumpetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TrumpetMesh"));
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	RootComponent = TrumpetMesh;
	
	TrumpetMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	TrumpetMesh->SetSimulatePhysics(true);

	CapsuleComponent->SetupAttachment(RootComponent);
    CapsuleComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CapsuleComponent->SetSimulatePhysics(false);
	
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;
	AudioComponent->bAllowSpatialization = true;
	AudioComponent->bOverrideAttenuation = true;
	
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

void ATrumpet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrumpet, bIsGrabbed);
	DOREPLIFETIME(ATrumpet, LastOwningCharacter);
}

void ATrumpet::OnGrab(bool isGrabbed, ACharacter* Parent)
{
	if (HasAuthority())
	{
		bIsGrabbed = isGrabbed;
		LastOwningCharacter = Parent;
		OnRep_Grabbed();
	}
}

void ATrumpet::OnRep_Grabbed()
{
	if (bIsGrabbed)
	{
		PlaySound();
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TrumpetMesh->SetSimulatePhysics(false);
		AttachToComponent(LastOwningCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("TrumpetSocket"));
	}
	else
	{
		StopSound();
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		TrumpetMesh->SetSimulatePhysics(true);
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		const FVector ForwardImpulse = LastOwningCharacter->GetActorForwardVector() * 500.0f;
		const FVector UpwardImpulse = FVector::UpVector * 300.0f;
		TrumpetMesh->AddImpulse(ForwardImpulse + UpwardImpulse);
	}
}

void ATrumpet::PlaySound() const
{
	if (!AudioComponent) return;

	AudioComponent->Play();
}

void ATrumpet::StopSound() const
{
	if (!AudioComponent) return;

	AudioComponent->Stop();
}