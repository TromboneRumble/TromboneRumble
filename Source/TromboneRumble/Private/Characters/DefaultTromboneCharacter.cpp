// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/DefaultPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Actors/SpotlightZone.h"
#include "Components/ActorComponents/AttackComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponents/InteractorComponent.h"
#include "Data/CharacterDataAsset.h"
#include "Framework/DefaultPlayerState.h"
#include "Items/InstrumentBase.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"

ADefaultTromboneCharacter::ADefaultTromboneCharacter()
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor"));
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
}

void ADefaultTromboneCharacter::Jump()
{
	const AItemBase* Instrument = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Instrument);
	
	if (bIsSprinting && !Instrument)
	{
		AttackComponent->Attack();
	}
	else
	{
		Super::Jump();
	}
}

void ADefaultTromboneCharacter::Move(const struct FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ADefaultTromboneCharacter::TryInteract()
{
	if (InteractorComponent) InteractorComponent->TryInteract(CurrentInteractionContext);
}

void ADefaultTromboneCharacter::Attack()
{
	const AItemBase* Instrument = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Instrument);
	
	if (AttackComponent && Instrument) AttackComponent->Attack();
}

void ADefaultTromboneCharacter::StartSprint()
{
	if (bIsSprinting) return; 

	bIsSprinting = true;
	Server_SetIsSprinting(true);

	if (CharacterData) GetCharacterMovement()->MaxWalkSpeed = CharacterData->SprintSpeed;
}

void ADefaultTromboneCharacter::StopSprint()
{
	if (!bIsSprinting) return;

	bIsSprinting = false;
	Server_SetIsSprinting(false);

	if (CharacterData) GetCharacterMovement()->MaxWalkSpeed = CharacterData->WalkSpeed;
}

void ADefaultTromboneCharacter::Rhythm(bool bIsPressed)
{
	const AItemBase* Instrument = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Instrument);

	if (!GetCachedRhythmActor() || !Instrument) return;

	if (bIsPressed)
	{
		CachedRhythmActor->DetectNotes();
	}
	else
	{
		CachedRhythmActor->DetectLongNoteEnd();
	}
}

EInstrumentType ADefaultTromboneCharacter::GetCurrentEquippedInstrumentType() const
{
	if (AItemBase* Instrument = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Instrument))
	{
		if (const AInstrumentBase* InstrumentBase = Cast<AInstrumentBase>(Instrument))
		{
			return InstrumentBase->GetInstrumentType();
		}
	}
	return EInstrumentType::Invalid;
}

void ADefaultTromboneCharacter::BeginPlay()
{
	Super::BeginPlay();

	OnRagdollDelegate.AddDynamic(this, &ThisClass::HandleOnRagdoll);
	
	EquipmentComponent->OnEquipmentChangedDelegate.AddDynamic(this, &ThisClass::HandleOnEquipmentChanged);
	constexpr EEquipmentSlotType TargetSlot = EEquipmentSlotType::Instrument;
	if (AItemBase* AlreadyEquippedItem = EquipmentComponent->GetItemInSlot(TargetSlot))
	{
		HandleOnEquipmentChanged(TargetSlot, AlreadyEquippedItem, nullptr);
	}
	
	if (IsLocallyControlled())
	{
		CameraBoom->TargetArmLength = CharacterData->TargetArmLength;
		CameraBoom->SetRelativeRotation(FRotator(CharacterData->CameraRelativeRotationPitch, 0.f, 0.f));
		CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, CharacterData->CameraRelativeLocationZ));
	
		CachedCharacterController = Cast<ADefaultPlayerController>(GetController());
		if (ARhythmActor* RhythmActor = GetCachedRhythmActor())
		{
			RhythmActor->OnNoteDetected.AddDynamic(this, &ThisClass::HandleOnNoteDetected);
		}

		InteractorComponent->OnInteractableAvailable.RemoveDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
		InteractorComponent->OnInteractableAvailable.AddDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
		InteractorComponent->OnInteractSuccessDelegate.AddDynamic(this, &ThisClass::HandleInteractSuccess);
	}
}

void ADefaultTromboneCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bIsSprinting);
}

void ADefaultTromboneCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;

	if (EquipmentComponent)
	{
		EquipmentComponent->InitializeOwner(this);
	}
	
	const ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
	if (PS && PS->EquippedInstrumentClass)
	{
		UWorld* World = GetWorld();
		if (!World) return;
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		AInstrumentBase* NewInstrument = World->SpawnActor<AInstrumentBase>(PS->EquippedInstrumentClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (NewInstrument)
		{
			EquipmentComponent->TryEquipItem(NewInstrument);
		}
	}
}

void ADefaultTromboneCharacter::Server_SetIsSprinting_Implementation(const bool bNewIsSprinting)
{
	if (bIsSprinting != bNewIsSprinting)
	{
		bIsSprinting = bNewIsSprinting;
	}
}

void ADefaultTromboneCharacter::Server_InteractItem_Implementation(AItemBase* InteractedItem)
{
	EquipmentComponent->TryEquipItem(InteractedItem);
}

void ADefaultTromboneCharacter::Server_RequestSpotlightBonus_Implementation()
{
	TArray<AActor*> OverlappingZones;
	GetOverlappingActors(OverlappingZones, ASpotlightZone::StaticClass());

	if (OverlappingZones.IsEmpty()) return;

	for (AActor* Actor : OverlappingZones)
	{
		if (ASpotlightZone* Zone = Cast<ASpotlightZone>(Actor))
		{
			if (Zone->TryAwardBonus(this))
			{
				Multicast_PlaySpotlightSuccessEffect();
				break; 
			}
		}
	}
}

void ADefaultTromboneCharacter::HandleInteractableAvailableChanged(bool bAvailable)
{
	if (ADefaultPlayerController* PC = CachedCharacterController.Get())
	{
		PC->ShowInteractionUI(bAvailable);
	}
}

void ADefaultTromboneCharacter::HandleInteractSuccess(AActor* InteractedActor)
{
	if (!IsValid(InteractedActor)) return;

	if (AItemBase* Item = Cast<AItemBase>(InteractedActor))
	{
		Server_InteractItem(Item);
	}
}

void ADefaultTromboneCharacter::HandleOnRagdoll()
{
	EquipmentComponent->TryUnequipItem(EEquipmentSlotType::Instrument);
}

void ADefaultTromboneCharacter::HandleOnEquipmentChanged(const EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (Slot == EEquipmentSlotType::Instrument)
	{
		if (NewItem)
		{
			if (const AInstrumentBase* Instrument = Cast<AInstrumentBase>(NewItem))
			{
				CurrentInteractionContext.bIsEquipped = true;
				if (GetCachedRhythmActor() && IsLocallyControlled())
				{
					CachedRhythmActor->ExecuteOnInstrumentPicked(Instrument->GetInstrumentType());
				}
			}
		}
		else
		{
			CurrentInteractionContext.bIsEquipped = false;
			if (GetCachedRhythmActor() && IsLocallyControlled())
			{
				CachedRhythmActor->ExecuteOnInstrumentPicked(EInstrumentType::Background);
			}
		}
	}
}

void ADefaultTromboneCharacter::HandleOnNoteDetected(ENoteResult NoteResult)
{
	if (NoteResult == ENoteResult::None || NoteResult == ENoteResult::Bad || NoteResult == ENoteResult::Invalid) return;
	
	if (!IsLocallyControlled()) return;
	
	if (HasAuthority())
	{
		Server_RequestSpotlightBonus_Implementation();
	}
	else
	{
		Server_RequestSpotlightBonus();
	}
}

void ADefaultTromboneCharacter::Multicast_PlaySpotlightSuccessEffect_Implementation()
{
	// TODO : 폭죽 이펙트 재생
	PRINT_WITH_CURRENT_CONTEXT("Spotlight Bonus Success!");
}

ARhythmActor* ADefaultTromboneCharacter::GetCachedRhythmActor()
{
	if (CachedRhythmActor.IsValid()) return CachedRhythmActor.Get();
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	if (ARhythmActor* FoundActor = Cast<ARhythmActor>(UGameplayStatics::GetActorOfClass(World, ARhythmActor::StaticClass())))
	{
		CachedRhythmActor = FoundActor;
		return CachedRhythmActor.Get();
	}
	return nullptr;
}
