// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Characters/DefaultTromboneCharacter.h"
#include "Characters/DefaultPlayerController.h"
#include "Components/ActorComponents/TromboneVOIPTalker.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/AttackComponent.h"
#include "Components/ActorComponents/CustomizationComponent.h"
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
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "UI/UserWidgets/InGame/InGameSpeakerWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/Defines.h"

ADefaultTromboneCharacter::ADefaultTromboneCharacter()
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	CustomizationComp = CreateDefaultSubobject<UCustomizationComponent>(TEXT("CustomizationComponent"));

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
	// 실행 순서:
	// 1) MID 초기화  2) LoadFromSaveData (저장 데이터 적용)
	// 3) Super::BeginPlay() → ReceiveBeginPlay() (Blueprint BeginPlay) 실행
	//    개발자가 BP에서 SetPartByKey/StepPart를 호출하면 저장 데이터를 덮어써서 디버깅 가능
	// 머티리얼 슬롯은 인덱스 하드코딩 대신 슬롯 이름("skin"/"face")으로 조회
	SkinMID = UCustomizationComponent::EnsureSlotMID(GetMesh(), TromboneMaterial::SkinSlotName);

	const int32 FaceIndex = GetMesh()->GetMaterialIndex(TromboneMaterial::FaceSlotName);
	if (FaceIndex != INDEX_NONE)
	{
		// MID 생성 전 원본 face 머티리얼 캐싱 (커스터마이징 복원용)
		OriginalFaceMaterial = GetMesh()->GetMaterial(FaceIndex);
		FaceMID = UCustomizationComponent::EnsureSlotMID(GetMesh(), TromboneMaterial::FaceSlotName);
	}

	if (CustomizationComp)
	{
		FCustomizationSaveData SaveData;
		if (IsLocallyControlled())
		{
			if (USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
				SaveData = SMS->LoadCustomization();
			CustomizationComp->LoadFromSaveData(SaveData);
			if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
				DPS->Server_SetCustomization(SaveData);
		}
		else
		{
			if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
			{
				SaveData = DPS->GetCustomizationData();
				CustomizationComp->LoadFromSaveData(SaveData);
			}
		}
	}

	Super::BeginPlay();

	// ~ Begin 구 TromboneCharacterBase::BeginPlay 후행부
	PlayFaceSequence(ECharacterFaceState::Blink);

	// 상태 전이는 베이스가 처리(Super::BeginPlay에서 바인딩됨). 여기서는 연출 핸들러만 구독한다
	OnStunStateChanged.AddDynamic(this, &ThisClass::HandleStunStateChanged);
	if (RagdollComponent)
	{
		RagdollComponent->OnRagdollStarted.AddDynamic(this, &ThisClass::HandleRagdollStartedVisuals);
		RagdollComponent->OnRagdollEnded.AddDynamic(this, &ThisClass::HandleRagdollEndedVisuals);
		RagdollComponent->OnRagdollPhysicsEnabled.AddDynamic(this, &ThisClass::HandleRagdollPhysicsEnabled);
	}

	BoundBounceTimeline();

	UpdateSkinFromPlayerState();
	ApplyFlagPhysics();
	// ~ End 구 TromboneCharacterBase::BeginPlay 후행부

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

		GetCharacterMovement()->MaxWalkSpeed = CharacterAttributes->GetMoveSpeed();

		CharacterAttributes->InitGroundFriction(GetCharacterMovement()->GroundFriction);
		CharacterAttributes->InitBrakingDeceleration(GetCharacterMovement()->BrakingDecelerationWalking);
		CharacterAttributes->InitLocomotionPlayRate(1.f);
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
	}
}

void ADefaultTromboneCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RetryVOIPRegistrationHandle);

	if (HasAuthority())
	{
		if (const UGameStateSubsystem* Sub = GetGameInstance()->GetSubsystem<UGameStateSubsystem>())
		{
			if (IsInGameLevelType(Sub->GetLevelState()))
			{
				EquipmentComponent->TryUnequipItem(EEquipmentSlotType::Weapon);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ADefaultTromboneCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (BounceTimeline.IsPlaying())
	{
		BounceTimeline.TickTimeline(DeltaSeconds);
	}
}

void ADefaultTromboneCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsSprinting);
	DOREPLIFETIME(ThisClass, SkinColor);
}

void ADefaultTromboneCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// ~ Begin 구 TromboneCharacterBase::PossessedBy
	UpdateSkinFromPlayerState();

	if (CustomizationComp)
	{
		if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
		{
			CustomizationComp->LoadFromSaveData(DPS->GetCustomizationData());
		}
	}
	// ~ End 구 TromboneCharacterBase::PossessedBy

	if (!HasAuthority()) return;

	// Server쪽에서 OnRep_PlayerState가 호출되지 않기 때문에 PossessedBy에서 호출
	TryRegisterVOIPTalker();

	SpawnAndEquipDefaultWeapon();
	SpawnAndEquipPreviouslyEquippedWeapon();
}

void ADefaultTromboneCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// ~ Begin 구 TromboneCharacterBase::OnRep_PlayerState
	UpdateSkinFromPlayerState();

	if (IsLocallyControlled())
	{
		if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
		{
			if (const USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
			{
				DPS->Server_SetCustomization(SMS->LoadCustomization());
			}
		}
	}
	else if (CustomizationComp)
	{
		if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
		{
			CustomizationComp->LoadFromSaveData(DPS->GetCustomizationData());
		}
	}
	// ~ End 구 TromboneCharacterBase::OnRep_PlayerState

	TryRegisterVOIPTalker();
}

void ADefaultTromboneCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (!IsLocallyControlled()) return;
	const USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	if (!SMS) return;

	const FCustomizationSaveData SaveData = SMS->LoadCustomization();
	if (CustomizationComp)
	{
		CustomizationComp->LoadFromSaveData(SaveData);
	}
	if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
	{
		DPS->Server_SetCustomization(SaveData);
	}
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

void ADefaultTromboneCharacter::ApplySkinColor(const FLinearColor InSkinColor)
{
	// 머리(leader). bApplySkinColorTint=false면 머티리얼 기본색 유지 (PlayerState 없는 더미)
	if (bApplySkinColorTint)
	{
		if (SkinMID)
		{
			SkinMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, InSkinColor);
		}
		if (FaceMID)
		{
			FaceMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, InSkinColor);
		}
	}
	// 몸통(costume)·안테나 follower 메시
	if (CustomizationComp)
	{
		CustomizationComp->ApplyPartsSkinColor(InSkinColor);
	}

	OnSkinColorChanged.Broadcast(InSkinColor);
}

void ADefaultTromboneCharacter::OnRep_SkinColor()
{
	ApplySkinColor(SkinColor);
}

void ADefaultTromboneCharacter::ApplyFaceMaterial(UMaterialInterface* Material)
{
	// nullptr 전달 시 BeginPlay에서 캐싱된 원본 머티리얼로 복원
	UMaterialInterface* Target = Material ? Material : OriginalFaceMaterial.Get();
	if (!Target) return;

	const int32 FaceIndex = GetMesh()->GetMaterialIndex(TromboneMaterial::FaceSlotName);
	if (FaceIndex == INDEX_NONE) return;

	GetMesh()->SetMaterial(FaceIndex, Target);
	FaceMID = GetMesh()->CreateAndSetMaterialInstanceDynamic(FaceIndex);
	if (FaceMID && bApplySkinColorTint)
	{
		// 현재 SkinColor를 새 MID에 재적용 (UpdateSkinFromPlayerState 전에 호출될 경우 초기값 Black이지만 이후 덮어써짐)
		// bApplySkinColorTint=false면 머티리얼 기본 BaseColor 유지 (PlayerState 없는 더미)
		FaceMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, SkinColor);
	}
}

void ADefaultTromboneCharacter::HandleRagdollStartedVisuals()
{
	PlayFaceSequence(ECharacterFaceState::Ragdoll);
}

void ADefaultTromboneCharacter::HandleRagdollEndedVisuals()
{
	PlayFaceSequence(ECharacterFaceState::Blink);
}

void ADefaultTromboneCharacter::HandleRagdollPhysicsEnabled()
{
	ApplyFlagPhysics();
}

void ADefaultTromboneCharacter::UpdateSkinFromPlayerState()
{
	if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
	{
		SkinColor = DPS->GetSkinColor();
		ApplySkinColor(SkinColor);
	}
}

void ADefaultTromboneCharacter::UpdateFaceExpression(ECharacterFaceType NewType)
{
	if (FaceMID)
	{
		FaceMID->SetScalarParameterValue(FaceExpressionParameterName, static_cast<float>(NewType));
	}
}

void ADefaultTromboneCharacter::BoundBounceTimeline()
{
	if (BounceCurve)
	{
		FOnTimelineVector ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleBounceProgress"));
		BounceTimeline.AddInterpVector(BounceCurve, ProgressFunction);
	}
}

void ADefaultTromboneCharacter::HandleBounceProgress(FVector Value)
{
	if (GetMesh())
	{
		GetMesh()->SetRelativeScale3D(Value);
	}
}

void ADefaultTromboneCharacter::PlayFaceSequence(const ECharacterFaceState TargetState)
{
	if (!CharacterData) return;

	if (const FCharacterFaceAnimationSequence* FaceAnimData = CharacterData->FaceSequences.Find(TargetState))
	{
		InternalPlayFaceSequence(FaceAnimData);
	}
}

void ADefaultTromboneCharacter::InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence)
{
	GetWorld()->GetTimerManager().ClearTimer(FaceSequenceTimerHandle);
	CurrentActiveSequence = *InSequence;
	CurrentSequenceStep = 0;
	ExecuteFaceStep();
}

void ADefaultTromboneCharacter::ExecuteFaceStep()
{
	if (CurrentActiveSequence.Sequence.Num() == 0) return;

	UpdateFaceExpression(CurrentActiveSequence.Sequence[CurrentSequenceStep]);
	CurrentSequenceStep++;

	if (CurrentSequenceStep < CurrentActiveSequence.Sequence.Num())
	{
		GetWorld()->GetTimerManager().SetTimer(FaceSequenceTimerHandle, this, &ThisClass::ExecuteFaceStep, CurrentActiveSequence.Interval, false);
	}
	else if (CurrentActiveSequence.bLoop)
	{
		CurrentSequenceStep = 0;

		float NextDelay = FMath::FRandRange(CurrentActiveSequence.MinLoopDelay, CurrentActiveSequence.MaxLoopDelay);
		if (NextDelay <= 0.0f) NextDelay = CurrentActiveSequence.Interval;

		GetWorld()->GetTimerManager().SetTimer(FaceSequenceTimerHandle, this, &ThisClass::ExecuteFaceStep, NextDelay, false);
	}
}

void ADefaultTromboneCharacter::ApplyFlagPhysics()
{
	if (!GetWorld() || !GetWorld()->GetGameInstance())
	{
		return;
	}

	if (const UGameStateSubsystem* GameStateSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UGameStateSubsystem>())
	{
		if (IsInGameLevelType(GameStateSubsystem->GetLevelState()))
		{
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			FPhysicalAnimationData FlagAnimData;
			FlagAnimData.bIsLocalSimulation = false;
			FlagAnimData.OrientationStrength = 10.0f;
			FlagAnimData.AngularVelocityStrength = 5.0f;
			FlagAnimData.PositionStrength = 10.0f;
			FlagAnimData.VelocityStrength = 0.0f;
			FlagAnimData.MaxAngularForce = 0.0f;
			FlagAnimData.MaxLinearForce = 0.0f;

			GetMesh()->SetAllBodiesBelowSimulatePhysics(TromboneBones::Flage, true, true);
			PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow(TromboneBones::Flage, FlagAnimData, true);
		}
	}
}

void ADefaultTromboneCharacter::OnBlockedStateChanged(const bool bBlocked)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !IsLocallyControlled())
	{
		return;
	}

	if (bBlocked)
	{
		DisableInput(PlayerController);
	}
	else
	{
		EnableInput(PlayerController);
	}
}

void ADefaultTromboneCharacter::HandleStunStateChanged(const bool bIsStunned)
{
	if (bIsStunned)
	{
		PlayFaceSequence(ECharacterFaceState::Stun);
		if (BounceCurve)
		{
			BounceTimeline.PlayFromStart();
		}
	}
	else
	{
		PlayFaceSequence(ECharacterFaceState::Blink);
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

	if (!IsLocallyControlled() && !VOIPTalker->IsRemoteTalkerRegistered())
	{
		GetWorldTimerManager().SetTimer(RetryVOIPRegistrationHandle,
			this, &ADefaultTromboneCharacter::TryRegisterVOIPTalker, 1.0f, false);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(RetryVOIPRegistrationHandle);
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

float ADefaultTromboneCharacter::GetLocomotionPlayRate() const
{
	return CharacterAttributes ? CharacterAttributes->GetLocomotionPlayRate() : 1.f;
}
