// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/DefaultPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Components/ActorComponents/AttackComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Components/ActorComponents/InteractorComponent.h"
#include "Components/ActorComponents/ClientToServerRelayComponent.h"
#include "Components/StaticMeshComponents/RingHitBoxComponent.h"
#include "Components/ActorComponents/RageComponent.h"
#include "Components/WidgetComponent.h"
#include "AbilitySystemComponent.h"
#include "Data/CharacterAttributeSet.h"
#include "Data/RhythmScoreAttributeSet.h"
#include "Data/CharacterDataAsset.h"
#include "Framework/DefaultPlayerState.h"
#include "Items/WeaponBase.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Items/InstrumentBase.h"

#include "Kismet/GameplayStatics.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Prototype/InGameWidget.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/DebugHelper.h"

ADefaultTromboneCharacter::ADefaultTromboneCharacter()
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), FName("pelvis"));
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
	ServerRelayComponent = CreateDefaultSubobject<UClientToServerRelayComponent>(TEXT("ServerRelayComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	RageComponent = CreateDefaultSubobject<URageComponent>(TEXT("RageComponent"));
	CharacterAttributes = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributes"));
	RhythmScoreAttributes = CreateDefaultSubobject<URhythmScoreAttributeSet>(TEXT("ScoreAttributeSet"));

	RingHitBoxComponent = CreateDefaultSubobject<URingHitBoxComponent>(TEXT("RingHitboxComponent"));
	if (RingHitBoxComponent)
	{
		RingHitBoxComponent->SetupAttachment(GetMesh());
	}

	ComboWidgetAnchorComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ComboWidgetAnchorComponent"));
	ComboWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ComboWidgetComponent"));
	if (ComboWidgetAnchorComponent && ComboWidgetComponent)
	{
		ComboWidgetAnchorComponent->SetupAttachment(GetMesh());
		ComboWidgetAnchorComponent->SetAbsolute(false,true,false);
		ComboWidgetComponent->SetupAttachment(ComboWidgetAnchorComponent);
		ComboWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
		ComboWidgetComponent->SetAbsolute(false, true, false);
		ComboWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ComboWidgetComponent->bReceivesDecals = 0;
		ComboWidgetComponent->SetCastShadow(false);
	}
}

void ADefaultTromboneCharacter::Jump()
{
	const AItemBase* Instrument = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Weapon);

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
		const FRotator Rotation = FRotator::ZeroRotator;
		//Ragdoll이 되며 Controller가 회전되어도 플레이어는 고정 축을 기준으로 움직이고 싶기 때문에 때문에 주석처리
		//const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ADefaultTromboneCharacter::TryInteract()
{
	if (InteractorComponent) InteractorComponent->TryInteract();
}

void ADefaultTromboneCharacter::Attack()
{
	const AItemBase* Weapon = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Weapon);

	if (AttackComponent && Weapon) AttackComponent->Attack();
}

void ADefaultTromboneCharacter::StartSprint()
{
	if (bIsSprinting) return;

	bIsSprinting = true;
	Server_SetIsSprinting(true);
	UpdateMaxWalkSpeed();
}

void ADefaultTromboneCharacter::StopSprint()
{
	if (!bIsSprinting) return;

	bIsSprinting = false;
	Server_SetIsSprinting(false);
	UpdateMaxWalkSpeed();
}

void ADefaultTromboneCharacter::Rhythm(bool bIsPressed)
{
	const AItemBase* Instrument = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Weapon);
	if (!Instrument) return;

	if (const AWeaponBase* Weapon = Cast<AWeaponBase>(Instrument))
	{
		if (Weapon->GetWeaponType() == EWeaponType::Headbutt)
		{
			return;
		}
	}

	if (!GetCachedRhythmActor()) return;

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
	if (AItemBase* Instrument = EquipmentComponent->GetItemInSlot(EEquipmentSlotType::Weapon))
	{
		if (const AWeaponBase* InstrumentBase = Cast<AWeaponBase>(Instrument))
		{
			return InstrumentBase->GetInstrumentType();
		}
	}
	return EInstrumentType::None;
}

void ADefaultTromboneCharacter::Equip(AItemBase* WeaponToEquip)
{
	if (EquipmentComponent)
	{
		EquipmentComponent->TryEquipItem(WeaponToEquip);
	}
}

void ADefaultTromboneCharacter::Unequip()
{
	if (!DefaultWeaponInstance) return;

	EquipmentComponent->TryUnequipItem(EEquipmentSlotType::Weapon);
	EquipmentComponent->TryEquipItem(DefaultWeaponInstance);
}

void ADefaultTromboneCharacter::BeginPlay()
{
	Super::BeginPlay();

	OnRagdollDelegate.AddDynamic(this, &ThisClass::HandleOnRagdoll);

	EquipmentComponent->OnEquipmentChangedDelegate.AddDynamic(this, &ThisClass::HandleOnEquipmentChanged);
	constexpr EEquipmentSlotType TargetSlot = EEquipmentSlotType::Weapon;
	if (AItemBase* AlreadyEquippedItem = EquipmentComponent->GetItemInSlot(TargetSlot))
	{
		HandleOnEquipmentChanged(TargetSlot, AlreadyEquippedItem, nullptr);
	}

	// GAS 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if (CharacterAttributes)
	{
		const float InitialSpeed = GetCharacterMovement()->MaxWalkSpeed;
		CharacterAttributes->InitMoveSpeed(InitialSpeed);

		// 혹시 OnRep 전에 바로 반영되도록 한 번 더 보정
		GetCharacterMovement()->MaxWalkSpeed = CharacterAttributes->GetMoveSpeed();
	}
	// ~GAS 초기화

	if (ComboWidgetComponent)
	{
		ComboWidgetComponent->SetVisibility(false);
	}

	if (IsLocallyControlled())
	{
		CameraBoom->TargetArmLength = CharacterData->TargetArmLength;
		CameraBoom->SetRelativeRotation(FRotator(CharacterData->CameraRelativeRotationPitch, 0.f, 0.f));
		CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, CharacterData->CameraRelativeLocationZ));

		CachedCharacterController = Cast<ADefaultPlayerController>(GetController());

		InteractorComponent->OnInteractableAvailable.RemoveDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
		InteractorComponent->OnInteractableAvailable.AddDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
		InteractorComponent->OnInteractSuccessDelegate.AddDynamic(this, &ThisClass::HandleInteractSuccess);

		ComboWidgetComponent->SetVisibility(true);

		// 가려진 캐릭터 실루엣을 위한 PostProcess 머티리얼을 로컬 카메라에만 블렌드
		if (OcclusionOverlayMaterial && FollowCamera)
		{
			FWeightedBlendable Blend(1.0f, OcclusionOverlayMaterial);
			FollowCamera->PostProcessSettings.WeightedBlendables.Array.Add(Blend);
		}
	}
}

void ADefaultTromboneCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (const UGameStateSubsystem* Sub = GetGameInstance()->GetSubsystem<UGameStateSubsystem>())
		{
			if (Sub->GetLevelState() == ELevelState::InGame)
			{
				EquipmentComponent->TryUnequipItem(EEquipmentSlotType::Weapon);
			}
		}
	}
	
	Super::EndPlay(EndPlayReason);
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

	SpawnAndEquipDefaultWeapon();
	SpawnAndEquipPreviouslyEquippedWeapon();
}

void ADefaultTromboneCharacter::Server_SetIsSprinting_Implementation(const bool bNewIsSprinting)
{
	if (bIsSprinting == bNewIsSprinting) return;

	bIsSprinting = bNewIsSprinting;
	UpdateMaxWalkSpeed();
}

void ADefaultTromboneCharacter::Server_InteractItem_Implementation(AItemBase* InteractedItem)
{
	EquipmentComponent->TryEquipItem(InteractedItem);
}

void ADefaultTromboneCharacter::UpdateMaxWalkSpeed()
{
	if (!CharacterData) return;

	const float BaseSpeed = bIsSprinting ? CharacterData->SprintSpeed : CharacterData->WalkSpeed;

	// MoveSpeed Attribute에 기본값 세팅
	// 나머지 로직은 GameplayEffect에서 처리
	if (HasAuthority() && CharacterAttributes)
	{
		CharacterAttributes->SetMoveSpeed(BaseSpeed);
	}
	// 로컬에서 움직임 즉시 반영
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// 현재 MoveSpeed Attribute가 있다면 그 값을, 없다면 BaseSpeed를 사용
		const float FinalSpeed = CharacterAttributes ? CharacterAttributes->GetMoveSpeed() : BaseSpeed;

		Move->MaxWalkSpeed = FinalSpeed;
	}
}

void ADefaultTromboneCharacter::HandleInteractableAvailableChanged(bool bAvailable)
{
}

void ADefaultTromboneCharacter::HandleInteractSuccess(AActor* InteractedActor)
{
	if (!IsValid(InteractedActor)) return;

	if (AItemBase* Item = Cast<AItemBase>(InteractedActor))
	{
		Server_InteractItem(Item);
	}

	if (const AInstrumentBase* InstrumentBase = Cast<AInstrumentBase>(InteractedActor))
	{
		if (ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>())
		{
			PS->AddScore(InstrumentBase->GetInstrumentPickUpScore(), EScoreType::InstrumentPickedUp);
		}
	}
}

void ADefaultTromboneCharacter::HandleOnRagdoll()
{
	Unequip();
}

void ADefaultTromboneCharacter::HandleOnEquipmentChanged(const EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem)
{
	if (Slot == EEquipmentSlotType::Weapon)
	{
		EInstrumentType NewType = EInstrumentType::Background;
		if (NewItem)
		{
			if (const AWeaponBase* Instrument = Cast<AWeaponBase>(NewItem))
			{
				NewType = Instrument->GetInstrumentType();
			}
		}

		if (IsLocallyControlled())
		{
			EInstrumentType OldType = EInstrumentType::None;
			if (const AWeaponBase* OldInstrument = Cast<AWeaponBase>(OldItem))
			{
				OldType = OldInstrument->GetInstrumentType();
			}

			if (const URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
			{
				RhythmSubsystem->OnInstrumentPicked.Broadcast(OldType, NewType);
			}

		}
	}
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

void ADefaultTromboneCharacter::SpawnAndEquipDefaultWeapon()
{
	if (!DefaultWeaponClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	DefaultWeaponInstance = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, SpawnParams);
	if (DefaultWeaponInstance)
	{
		AttackComponent->SetDefaultWeaponInstance(DefaultWeaponInstance);
		EquipmentComponent->TryEquipItem(DefaultWeaponInstance);
	}
}

void ADefaultTromboneCharacter::SpawnAndEquipPreviouslyEquippedWeapon()
{
	const ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();

	if (PS && PS->EquippedWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		if (AWeaponBase* EquippedWeapon = GetWorld()->SpawnActor<AWeaponBase>(PS->EquippedWeaponClass, GetActorLocation(), GetActorRotation(), SpawnParams))
		{
			EquipmentComponent->TryEquipItem(EquippedWeapon);
		}
	}
}