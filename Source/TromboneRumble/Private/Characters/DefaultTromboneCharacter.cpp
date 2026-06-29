// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/DefaultPlayerController.h"
#include "Components/ActorComponents/TromboneVOIPTalker.h"
#include "GameFramework/PlayerState.h"
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
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "AbilitySystemComponent.h"
#include "Data/CharacterAttributeSet.h"
#include "Data/RhythmScoreAttributeSet.h"
#include "Data/CharacterDataAsset.h"
#include "Framework/DefaultPlayerState.h"
#include "Items/WeaponBase.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Components/ActorComponents/InterpolateSpringArmComponent.h"
#include "Items/InstrumentBase.h"
#include "UI/UserWidgets/InGame/InGameSpeakerWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/GameStateSubsystem.h"

const FName ADefaultTromboneCharacter::SilhouetteColorParamName(TEXT("SilhouetteColor"));

ADefaultTromboneCharacter::ADefaultTromboneCharacter()
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<UInterpolateSpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), TromboneBones::Pelvis);
	
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

	// Voice chat: positional (3D) playback for in-game characters.
	VOIPTalker = CreateDefaultSubobject<UTromboneVOIPTalker>(TEXT("VOIPTalker"));
	if (VOIPTalker)
	{
		VOIPTalker->bPositional = true;
	}

	SpeakerIndicatorComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SpeakerIndicatorComponent"));
	if (SpeakerIndicatorComponent)
	{
		SpeakerIndicatorComponent->SetupAttachment(GetMesh(), FName("head"));
		SpeakerIndicatorComponent->SetWidgetSpace(EWidgetSpace::Screen);
		SpeakerIndicatorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SpeakerIndicatorComponent->bReceivesDecals = 0;
		SpeakerIndicatorComponent->SetCastShadow(false);
		SpeakerIndicatorComponent->SetVisibility(true);
		SpeakerIndicatorComponent->SetHiddenInGame(false);
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

void ADefaultTromboneCharacter::OnCameraZoom(float WheelDelta)
{
	if (!IsLocallyControlled() || !CharacterData) return;

	// 휠 업(+) → 줌 인(레벨 감소), 휠 다운(-) → 줌 아웃(레벨 증가)
	const int32 Step = (WheelDelta > 0.f) ? -1 : 1;
	const int32 NewLevel = FMath::Clamp(CurrentZoomLevel + Step, 1, 3);
	if (NewLevel == CurrentZoomLevel) return;
	CurrentZoomLevel = NewLevel;

	switch (CurrentZoomLevel)
	{
	case 1:
		DesiredArmLength = CharacterData->CameraArmLengthLevel1;
		DesiredBoomRotation = FRotator(CharacterData->CameraPitchLevel1, 0.f, 0.f);
		break;
	case 3:
		DesiredArmLength = CharacterData->CameraArmLengthLevel3;
		DesiredBoomRotation = FRotator(CharacterData->CameraPitchLevel3, 0.f, 0.f);
		break;
	case 2:
	default:
		DesiredArmLength = CharacterData->CameraArmLengthLevel2;
		DesiredBoomRotation = FRotator(CharacterData->CameraPitchLevel2, 0.f, 0.f);
		break;
	}
}


void ADefaultTromboneCharacter::Server_SetSpeaking_Implementation(bool bSpeaking)
{
	// Forward to all clients (including sender) so every viewport toggles in lockstep.
	Multicast_SetSpeaking(bSpeaking);
}

void ADefaultTromboneCharacter::Multicast_SetSpeaking_Implementation(bool bSpeaking)
{
	// Defensive re-bind of OwnerPlayer with a forced cycle. UWidgetComponent::SetOwnerPlayer
	// only triggers RemoveWidgetFromScreen + re-add when the player pointer actually changes.
	// On the host, the screen-space widget for a client-controlled pawn can end up with a
	// stale bAddedToScreen=true bound to the wrong (or null) screen layer — re-calling
	// SetOwnerPlayer with the same player would be a no-op in that case. Cycling through
	// nullptr forces RemoveWidgetFromScreen so the next tick's UpdateWidgetOnScreen re-adds
	// the widget to the correct local player's screen layer.
	if (SpeakerIndicatorComponent)
	{
		ULocalPlayer* HostLocalPlayer = nullptr;
		if (UGameInstance* GI = GetGameInstance())
		{
			const TArray<ULocalPlayer*>& LocalPlayers = GI->GetLocalPlayers();
			if (LocalPlayers.Num() > 0)
			{
				HostLocalPlayer = LocalPlayers[0];
			}
		}

		if (HostLocalPlayer)
		{
			SpeakerIndicatorComponent->SetOwnerPlayer(nullptr);
			SpeakerIndicatorComponent->SetOwnerPlayer(HostLocalPlayer);
			SpeakerIndicatorComponent->MarkRenderStateDirty();
		}
	}

	// 표시 로직은 UI(InGameSpeakerWidget)가 담당. 여기서는 PTT 상태만 VOIPTalker로 위임.
	if (VOIPTalker)
	{
		VOIPTalker->SetPushToTalkSpeaking(bSpeaking);
	}
}

void ADefaultTromboneCharacter::BeginPlay()
{
	Super::BeginPlay();

	TryRegisterVOIPTalker();

	//멀티플레이어 환경에서 Widget을 생성 및 Owner지정
	if (SpeakerIndicatorComponent)
	{
		ULocalPlayer* BoundLocalPlayer = nullptr;
		if (UGameInstance* GI = GetGameInstance())
		{
			const TArray<ULocalPlayer*>& LocalPlayers = GI->GetLocalPlayers();
			if (LocalPlayers.Num() > 0)
			{
				BoundLocalPlayer = LocalPlayers[0];
			}
		}
		if (!BoundLocalPlayer)
		{
			if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
			{
				BoundLocalPlayer = LocalPC->GetLocalPlayer();
			}
		}

		if (BoundLocalPlayer)
		{
			SpeakerIndicatorComponent->SetOwnerPlayer(BoundLocalPlayer);
			SpeakerIndicatorComponent->MarkRenderStateDirty();
		}

		SpeakerIndicatorComponent->InitWidget();
		if (UInGameSpeakerWidget* SpeakerWidget = Cast<UInGameSpeakerWidget>(SpeakerIndicatorComponent->GetUserWidgetObject()))
		{
			SpeakerWidget->Init(VOIPTalker);
		}
	}

	if (RagdollComponent)
	{
		RagdollComponent->OnRagdollStarted.AddDynamic(this, &ThisClass::Unequip);
	}

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
		CurrentZoomLevel = 2;
		DesiredArmLength = CharacterData->CameraArmLengthLevel2;
		DesiredBoomRotation = FRotator(CharacterData->CameraPitchLevel2, 0.f, 0.f);
		CameraBoom->TargetArmLength = DesiredArmLength;
		CameraBoom->SetRelativeRotation(DesiredBoomRotation);
		CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, CharacterData->CameraRelativeLocationZ));

		CachedCharacterController = Cast<ADefaultPlayerController>(GetController());

		InteractorComponent->OnInteractableAvailable.RemoveDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
		InteractorComponent->OnInteractableAvailable.AddDynamic(this, &ThisClass::HandleInteractableAvailableChanged);
		InteractorComponent->OnInteractSuccessDelegate.AddDynamic(this, &ThisClass::HandleInteractSuccess);

		ComboWidgetComponent->SetVisibility(true);

		// 가려진 캐릭터 실루엣을 위한 PostProcess 머티리얼을 로컬 카메라에만 블렌드
		// 초기 weight=0.0 (OFF); CheckXRayOcclusion() 타이머가 XRayBlocker 감지 시 1.0으로 올림
		// 실루엣 색상을 로컬 플레이어 피부색으로 주입하기 위해 동적 인스턴스(MID)를 블렌드한다.
		if (OcclusionOverlayMaterial && FollowCamera)
		{
			OcclusionOverlayMID = UMaterialInstanceDynamic::Create(OcclusionOverlayMaterial, this);
			if (OcclusionOverlayMID)
			{
				// 현재 피부색으로 초기화 (색이 이미 도착한 경우 대비. 이후 ApplySkinColor에서 갱신)
				OcclusionOverlayMID->SetVectorParameterValue(SilhouetteColorParamName, GetSkinColor());

				FWeightedBlendable Blend(0.0f, OcclusionOverlayMID);
				FollowCamera->PostProcessSettings.WeightedBlendables.Array.Add(Blend);

				// 카메라→캐릭터 트레이스: XRayBlocker 감지 시 X-Ray ON
				GetWorldTimerManager().SetTimer(
					XRayTraceTimerHandle,
					this,
					&ThisClass::CheckXRayOcclusion,
					0.05f,
					true);
			}
		}
	}
}

void ADefaultTromboneCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(XRayTraceTimerHandle);

	if (HasAuthority())
	{
		if (const UGameStateSubsystem* Sub = GetGameInstance()->GetSubsystem<UGameStateSubsystem>())
		{
			if (Sub->GetLevelState() == ELevelType::InGame)
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

	// Server쪽에서 OnRep_PlayerState가 호출되지 않기 때문에 PossessedBy에서 호출
	TryRegisterVOIPTalker();

	SpawnAndEquipDefaultWeapon();
	SpawnAndEquipPreviouslyEquippedWeapon();
}

void ADefaultTromboneCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	TryRegisterVOIPTalker();
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

void ADefaultTromboneCharacter::CheckXRayOcclusion()
{
	if (!OcclusionOverlayMaterial || !FollowCamera) return;

	// 카메라 위치 → 캐릭터 중심까지 멀티 트레이스
	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End   = GetActorLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> Hits;
	GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_Visibility, Params);

	// 트레이스 결과 중 XRayBlocker 태그가 있는 액터가 하나라도 있으면 X-Ray ON
	bool bXRayActive = false;
	for (const FHitResult& Hit : Hits)
	{
		if (Hit.GetActor() && Hit.GetActor()->ActorHasTag(FName("XRayBlocker")))
		{
			bXRayActive = true;
			break;
		}
	}

	// blendable 배열에서 OcclusionOverlayMID를 찾아 weight 업데이트
	const float NewWeight = bXRayActive ? 1.0f : 0.0f;
	for (FWeightedBlendable& Blendable : FollowCamera->PostProcessSettings.WeightedBlendables.Array)
	{
		if (Blendable.Object == OcclusionOverlayMID)
		{
			Blendable.Weight = NewWeight;
			break;
		}
	}
}

void ADefaultTromboneCharacter::ApplySkinColor(const FLinearColor InSkinColor) const
{
	Super::ApplySkinColor(InSkinColor);

	// 로컬 플레이어 카메라에만 존재하는 X-Ray 실루엣 MID 색상을 피부색으로 갱신
	if (OcclusionOverlayMID)
	{
		OcclusionOverlayMID->SetVectorParameterValue(SilhouetteColorParamName, InSkinColor);
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

void ADefaultTromboneCharacter::TryRegisterVOIPTalker()
{
	if (!VOIPTalker)
	{
		return;
	}

	APlayerState* PS = GetPlayerState();
	if (!PS)
	{
		return;
	}

	VOIPTalker->RegisterTalker(PS);
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
