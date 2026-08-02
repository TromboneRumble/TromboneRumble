// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/TromboneCharacterBase.h"
#include "NiagaraComponent.h"
#include "Components/CapsuleComponent.h"
#include "AkComponent.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Data/CharacterDataAsset.h"
#include "Framework/DefaultPlayerState.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/Defines.h"

ATromboneCharacterBase::ATromboneCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	CustomizationComp = CreateDefaultSubobject<UCustomizationComponent>(TEXT("CustomizationComponent"));
	PhysicalAnimationComp = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimationComponent"));
	RagdollComponent = CreateDefaultSubobject<UTromboneRagdollComponent>(TEXT("RagdollComponent"));
	StunNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StunNiagaraComponent"));
	if (StunNiagaraComponent)
	{
		StunNiagaraComponent->SetupAttachment(GetMesh());
		StunNiagaraComponent->bAutoActivate = false;
	}
	AkSoundComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkSoundComponent"));
	if (AkSoundComponent)
	{
		AkSoundComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
		AkSoundComponent->OcclusionRefreshInterval = 0.f;
	}
	
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->InitCapsuleSize(42.f, 78.0f);
		CapsuleComp->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CapsuleComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
		CapsuleComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap); // Object Channel 1 : Weapon
		CapsuleComp->CanCharacterStepUpOn = ECB_No;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		MeshComp->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -CapsuleHalfHeight), FRotator(0.0f, -90.0f, 0.0f));
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
		MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		MeshComp->SetHiddenInGame(false);
	}
}

void ATromboneCharacterBase::ApplySkinColor(const FLinearColor InSkinColor)
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

void ATromboneCharacterBase::AddInputBlock(const EInputBlockReason Reason)
{
	const uint8 OldMask = InputBlockMask;
	InputBlockMask |= static_cast<uint8>(Reason);

	if (OldMask == 0 && InputBlockMask != 0)
	{
		ApplyEngineInputEnabled(false);
	}
}

void ATromboneCharacterBase::RemoveInputBlock(const EInputBlockReason Reason)
{
	const uint8 OldMask = InputBlockMask;
	InputBlockMask &= ~static_cast<uint8>(Reason);

	if (OldMask != 0 && InputBlockMask == 0)
	{
		ApplyEngineInputEnabled(true);
	}
}

void ATromboneCharacterBase::ApplyEngineInputEnabled(const bool bEnable)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			if (bEnable)
			{
				EnableInput(PlayerController);
			}
			else
			{
				DisableInput(PlayerController);
			}
		}
	}
}

void ATromboneCharacterBase::Server_SetInputEnabled(const bool bEnable)
{
	if (!HasAuthority() || bInputEnabled == bEnable)
	{
		return;
	}

	bInputEnabled = bEnable;
	OnRep_InputEnabled();
}

void ATromboneCharacterBase::OnRep_InputEnabled()
{
	if (bInputEnabled)
	{
		RemoveInputBlock(EInputBlockReason::ServerLock);
	}
	else
	{
		AddInputBlock(EInputBlockReason::ServerLock);
	}
}

void ATromboneCharacterBase::BeginPlay()
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

	PlayFaceSequence(ECharacterFaceState::Blink);

	if (RagdollComponent)
	{
		RagdollComponent->OnRagdollStarted.AddDynamic(this, &ThisClass::HandleRagdollStarted);
		RagdollComponent->OnRagdollEnded.AddDynamic(this, &ThisClass::HandleRagdollEnded);
		RagdollComponent->OnRagdollPhysicsEnabled.AddDynamic(this, &ThisClass::HandleRagdollPhysicsEnabled);
	}

	PhysicalAnimationComp->SetSkeletalMeshComponent(GetMesh());

	if (IsLocallyControlled())
	{
		// Sound Listener의 기본 설정을 카메라->Player로 변경
		if (AkSoundComponent)
		{
			TArray<UAkComponent*> Listeners;
			Listeners.Add(AkSoundComponent);
			AkSoundComponent->SetListeners(Listeners);
		}
	}

	SetupCharacterData();
	BoundBounceTimeline();

	UpdateSkinFromPlayerState();
	ApplyFlagPhysics();
}

void ATromboneCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ATromboneCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (BounceTimeline.IsPlaying())
	{
		BounceTimeline.TickTimeline(DeltaSeconds);
	}
}

void ATromboneCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bIsStun);
	DOREPLIFETIME(ThisClass, bIsInvincible);
	DOREPLIFETIME(ThisClass, bInputEnabled);
	DOREPLIFETIME(ThisClass, SkinColor);
}

void ATromboneCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UpdateSkinFromPlayerState();

	if (CustomizationComp)
	{
		if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
		{
			CustomizationComp->LoadFromSaveData(DPS->GetCustomizationData());
		}
	}
}

void ATromboneCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

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
}

void ATromboneCharacterBase::OnRep_Controller()
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

void ATromboneCharacterBase::OnHitReceived_Implementation(const FHitData& HitData)
{
	if (!HasAuthority()) return;

	if (bIsInvincible || bIsStun || IsRagdoll()) return;

	const FVector KnockbackVel = CalculateKnockbackVelocity(HitData);

	switch (HitData.HitReaction)
	{
		case EHitReactionType::Ragdoll:
			if (RagdollComponent) RagdollComponent->StartRagdoll(KnockbackVel);
			break;
		
		case EHitReactionType::Stun:
			OnStun();
			LaunchCharacter(KnockbackVel, true, true);
			Client_ApplyKnockback(KnockbackVel);
			break;
		
		case EHitReactionType::None:
			; // intentional fall through
		
		default:
			break;
	}
}

FVector ATromboneCharacterBase::CalculateKnockbackVelocity(const FHitData& HitData) const
{
	// 폭발형 히트: 폭심에서 바깥으로 방사형
	if (HitData.ExplosionStrength > 0.f)
	{
		return (GetActorLocation() - HitData.ImpactPoint).GetSafeNormal() * HitData.ExplosionStrength;
	}

	// 일반 히트: 수평 방향 × 수평 힘 + 상향 × 수직 힘 (호출자가 준 방향의 수직 성분은 무시)
	FVector HorizontalDir = HitData.HitDirection;
	HorizontalDir.Z = 0.f;
	return HorizontalDir.GetSafeNormal() * HitData.KnockbackForce + FVector::UpVector * HitData.KnockbackUpForce;
}

void ATromboneCharacterBase::Client_ApplyKnockback_Implementation(const FVector KnockbackVelocity)
{
	if (HasAuthority())
	{
		return;
	}

	LaunchCharacter(KnockbackVelocity, true, true);
}

void ATromboneCharacterBase::OnRep_SkinColor()
{
	ApplySkinColor(SkinColor);
}

void ATromboneCharacterBase::ApplyFaceMaterial(UMaterialInterface* Material)
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

void ATromboneCharacterBase::SetupCharacterData() const
{
	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	GetCharacterMovement()->NetworkMaxSmoothUpdateDistance = 128.f;
	GetCharacterMovement()->NetworkNoSmoothUpdateDistance = 384.f;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	
	if (CharacterData)
	{
		// Ground
		GetCharacterMovement()->MaxWalkSpeed = CharacterData->WalkSpeed;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, CharacterData->RotationRate, 0.0f);

		// Air
		GetCharacterMovement()->JumpZVelocity = CharacterData->JumpZVelocity;
		GetCharacterMovement()->AirControl = CharacterData->AirControl;

		// Inertia
		GetCharacterMovement()->GravityScale = CharacterData->GravityScale;
		GetCharacterMovement()->MaxAcceleration = CharacterData->MaxAcceleration;
		GetCharacterMovement()->BrakingDecelerationWalking = CharacterData->BrakingDecelerationWalking;
		GetCharacterMovement()->GroundFriction = CharacterData->GroundFriction;
	}
}

bool ATromboneCharacterBase::IsRagdoll() const
{
	return RagdollComponent ? RagdollComponent->IsRagdoll() : false;
}

void ATromboneCharacterBase::HandleRagdollStarted()
{
	if (HasAuthority() && bIsStun)
	{
		GetWorld()->GetTimerManager().ClearTimer(OnHitTimerHandle);
		bIsStun = false;
		OnRep_IsStun();
	}

	/** TODO : UCharacterAnimInstance::OnGetUpMontageEnded에서 래그돌 입력 차단을 해제하는데, 여기서 콜백을 넘겨주는 식으로 개선 못하나? */
	AddInputBlock(EInputBlockReason::Ragdoll);
	PlayFaceSequence(ECharacterFaceState::Ragdoll);
	if (AkSoundComponent && RagdollBooSound)
	{
		AkSoundComponent->PostAkEvent(RagdollBooSound, 0, FOnAkPostEventCallback());
	}
}

void ATromboneCharacterBase::HandleRagdollEnded()
{
	if (HasAuthority())
	{
		bIsInvincible = true;
		OnRep_IsInvincible();

		GetWorld()->GetTimerManager().SetTimer(
			InvincibilityTimerHandle,
			[this]()
			{
				bIsInvincible = false;
				OnRep_IsInvincible();
			},
			CharacterData->InvincibilityDurationAfterRagdoll,
			false
		);
	}
	
	PlayFaceSequence(ECharacterFaceState::Blink);
}

void ATromboneCharacterBase::HandleRagdollPhysicsEnabled()
{
	ApplyFlagPhysics();
}

void ATromboneCharacterBase::OnStun()
{
	if (!HasAuthority()) return;

	if (IsRagdoll() || bIsStun) return;

	bIsStun = true;
	OnRep_IsStun();

	GetWorld()->GetTimerManager().SetTimer(
		OnHitTimerHandle, 
		this, 
		&ThisClass::EndStun, 
		CharacterData->StunDuration, 
		false
	);
}

void ATromboneCharacterBase::EndStun()
{
	if (!HasAuthority()) return;

	bIsStun = false;
	OnRep_IsStun();
	
	bIsInvincible = true;
	OnRep_IsInvincible();
	
	GetWorld()->GetTimerManager().SetTimer(
		InvincibilityTimerHandle, 
		[this]()
		{
			bIsInvincible = false;
			OnRep_IsInvincible();
		}, 
		CharacterData->InvincibilityDurationAfterStun, 
		false
	);
}

void ATromboneCharacterBase::ApplyStun()
{
	StopAnimMontage();
	AddInputBlock(EInputBlockReason::Stun);
}

void ATromboneCharacterBase::UnapplyStun()
{
	RemoveInputBlock(EInputBlockReason::Stun);

	if (IsRagdoll())
	{
		return;
	}

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ATromboneCharacterBase::UpdateSkinFromPlayerState()
{
	if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
	{
		SkinColor = DPS->GetSkinColor();
		ApplySkinColor(SkinColor);
	}
}

void ATromboneCharacterBase::UpdateFaceExpression(ECharacterFaceType NewType)
{
	if (FaceMID)
	{
		FaceMID->SetScalarParameterValue(FaceExpressionParameterName, static_cast<float>(NewType));
	}
}

void ATromboneCharacterBase::BoundBounceTimeline()
{
	if (BounceCurve)
	{
		FOnTimelineVector ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleBounceProgress"));
		BounceTimeline.AddInterpVector(BounceCurve, ProgressFunction);
	}
}

void ATromboneCharacterBase::HandleBounceProgress(FVector Value)
{
	if (GetMesh())
	{
		GetMesh()->SetRelativeScale3D(Value);
	}
}

void ATromboneCharacterBase::Server_DebugStun_Implementation()
{
	if (bIsStun)
	{
		GetWorld()->GetTimerManager().ClearTimer(OnHitTimerHandle);
		EndStun();
	}
	else
	{
		OnStun();
	}
}

void ATromboneCharacterBase::Server_DebugRagdoll_Implementation()
{
	if (RagdollComponent)
	{
		RagdollComponent->StartRagdoll();
	}
}

void ATromboneCharacterBase::PlayFaceSequence(const ECharacterFaceState TargetState)
{
	if (!CharacterData) return;

	if (const FCharacterFaceAnimationSequence* FaceAnimData = CharacterData->FaceSequences.Find(TargetState))
	{
		InternalPlayFaceSequence(FaceAnimData);
	}
}

void ATromboneCharacterBase::InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence)
{
	GetWorld()->GetTimerManager().ClearTimer(FaceSequenceTimerHandle);
	CurrentActiveSequence = *InSequence;
	CurrentSequenceStep = 0;
	ExecuteFaceStep();
}

void ATromboneCharacterBase::ExecuteFaceStep()
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

void ATromboneCharacterBase::ApplyFlagPhysics()
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

void ATromboneCharacterBase::OnRep_IsStun()
{
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (bIsStun)
	{
		ApplyStun();
		PlayFaceSequence(ECharacterFaceState::Stun);
		if (BounceCurve)
		{
			BounceTimeline.PlayFromStart();
		}
		if (StunNiagaraComponent)
		{
			StunNiagaraComponent->DeactivateImmediate();
			StunNiagaraComponent->Activate(true);
		}
		if (AkSoundComponent && StunNiagaraSound)
		{
			StunNiagaraPlayingID = AkSoundComponent->PostAkEvent(StunNiagaraSound, 0, FOnAkPostEventCallback());
		}
	}
	else
	{
		UnapplyStun();
		PlayFaceSequence(ECharacterFaceState::Blink);
		if (StunNiagaraComponent)
		{
			StunNiagaraComponent->DeactivateImmediate();
		}
		if (AudioDevice && StunNiagaraPlayingID != 0)
		{
			AudioDevice->StopPlayingID(StunNiagaraPlayingID, 0, AkCurveInterpolation_Linear);
			StunNiagaraPlayingID = 0;
		}
	}
	OnStunStateChanged.Broadcast(bIsStun);
}

void ATromboneCharacterBase::OnRep_IsInvincible()
{
	if (bIsInvincible)
	{
		OnInvincibleDelegate.Broadcast();
	}
	else
	{
		EndInvincibleDelegate.Broadcast();
	}
}