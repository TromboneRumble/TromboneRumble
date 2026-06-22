// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/TromboneCharacterBase.h"

#include "NiagaraComponent.h"
#include "Animation/CharacterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "AkComponent.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Data/CharacterDataAsset.h"
#include "Framework/DefaultPlayerState.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/Defines.h"
#include "Utilities/DebugHelper.h"

ATromboneCharacterBase::ATromboneCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	CustomizationComp = CreateDefaultSubobject<UCustomizationComponent>(TEXT("CustomizationComponent"));
	PhysicalAnimationComp = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimationComponent"));
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
	
	InitCharacter();
}

void ATromboneCharacterBase::ApplyOccludedStencil(UPrimitiveComponent* Prim)
{
	if (!Prim) return;
	Prim->SetRenderCustomDepth(true);
	Prim->SetCustomDepthStencilValue(TromboneRender::CHARACTER_OCCLUDED_STENCIL);
}

void ATromboneCharacterBase::ClearOccludedStencil(UPrimitiveComponent* Prim)
{
	if (!Prim) return;
	Prim->SetRenderCustomDepth(false);
}

void ATromboneCharacterBase::ApplyOccludedStencilToActor(AActor* Actor)
{
	if (!Actor) return;
	TArray<UPrimitiveComponent*> Prims;
	Actor->GetComponents<UPrimitiveComponent>(Prims);
	for (UPrimitiveComponent* Prim : Prims)
	{
		ApplyOccludedStencil(Prim);
	}
}

void ATromboneCharacterBase::ClearOccludedStencilFromActor(AActor* Actor)
{
	if (!Actor) return;
	TArray<UPrimitiveComponent*> Prims;
	Actor->GetComponents<UPrimitiveComponent>(Prims);
	for (UPrimitiveComponent* Prim : Prims)
	{
		ClearOccludedStencil(Prim);
	}
}
void ATromboneCharacterBase::ApplySkinColor(const FLinearColor InSkinColor) const
{
	// 머리(leader)
	if (SkinMID)
	{
		SkinMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, InSkinColor);
	}
	if (FaceMID)
	{
		FaceMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, InSkinColor);
	}
	// 몸통(costume)·안테나 follower 메시
	if (CustomizationComp)
	{
		CustomizationComp->ApplyPartsSkinColor(InSkinColor);
	}
}

void ATromboneCharacterBase::SetPlayerInput(const bool bShouldEnable)
{
	bIsCanProcessInput = bShouldEnable;

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			if (bShouldEnable)
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

	// 로컬 플레이어 캐릭터만 X-Ray stencil=252 적용 (원격 캐릭터는 X-Ray 미표시)
	if (IsLocallyControlled())
	{
		ApplyOccludedStencil(GetMesh());
	}

	SetupCharacterData();
	BoundBounceTimeline();

	UpdateSkinFromPlayerState();
	ApplyFlagPhysics();
}

void ATromboneCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(OnHitTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(FaceSequenceTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandler_DelayedSavePostSnapshot);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandler_InternalUnapplyRagdoll);
	
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
	
	DOREPLIFETIME(ThisClass, bIsRagdoll);
	DOREPLIFETIME(ThisClass, bIsStun);
	DOREPLIFETIME(ThisClass, bIsInvincible);
	DOREPLIFETIME(ThisClass, SkinColor);
}

void ATromboneCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UpdateSkinFromPlayerState();

	if (CustomizationComp)
		if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
			CustomizationComp->LoadFromSaveData(DPS->GetCustomizationData());
}

void ATromboneCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	UpdateSkinFromPlayerState();

	if (IsLocallyControlled())
	{
		if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
			if (USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
				DPS->Server_SetCustomization(SMS->LoadCustomization());
	}
	else if (CustomizationComp)
	{
		if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
			CustomizationComp->LoadFromSaveData(DPS->GetCustomizationData());
	}
}

void ATromboneCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (!IsLocallyControlled()) return;
	USaveManagerSubsystem* SMS = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	if (!SMS) return;

	FCustomizationSaveData SaveData = SMS->LoadCustomization();
	if (CustomizationComp)
		CustomizationComp->LoadFromSaveData(SaveData);
	if (ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
		DPS->Server_SetCustomization(SaveData);
}

void ATromboneCharacterBase::OnHitReceived_Implementation(const FHitData& HitData)
{
	if (!HasAuthority()) return;

	if (bIsInvincible || bIsStun || bIsRagdoll) return;

	switch (HitData.HitReaction)
	{
	case EHitReactionType::Ragdoll:
		OnRagdoll();
		break;
	case EHitReactionType::Stun:
		OnStun();
		break;
	case EHitReactionType::None:
	default:
		break;
	}

	LaunchCharacter(HitData.HitDirection * HitData.KnockbackForce, true, true);
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
	if (FaceMID)
	{
		// 현재 SkinColor를 새 MID에 재적용 (UpdateSkinFromPlayerState 전에 호출될 경우 초기값 Black이지만 이후 덮어써짐)
		FaceMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, SkinColor);
	}
}

void ATromboneCharacterBase::InitCharacter()
{
	SetupCapsuleComponent();
	SetupSkeletalMeshComponent();
}

void ATromboneCharacterBase::SetupCapsuleComponent()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 78.0f);
	GetCapsuleComponent()->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap); // Object Channel 1 : Weapon
	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_No;
}

void ATromboneCharacterBase::SetupSkeletalMeshComponent()
{
	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -CapsuleHalfHeight), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetHiddenInGame(false);
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

void ATromboneCharacterBase::OnRagdoll()
{
	if (!HasAuthority()) return;
	
	if (bIsRagdoll) return;

	if (bIsStun)
	{
		GetWorld()->GetTimerManager().ClearTimer(OnHitTimerHandle);
		bIsStun = false;
		OnRep_IsStun();
	}
	
	bIsRagdoll = true;
	OnRep_IsRagdoll();
	
	GetWorld()->GetTimerManager().SetTimer(
		OnHitTimerHandle, 
		this, 
		&ThisClass::EndRagdoll, 
		CharacterData->RagdollDuration, 
		false
	);
}

void ATromboneCharacterBase::EndRagdoll()
{
	if (!HasAuthority()) return;

	bIsRagdoll = false;
	OnRep_IsRagdoll();
	
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

void ATromboneCharacterBase::OnStun()
{
	if (!HasAuthority()) return;
    
	if (bIsRagdoll || bIsStun) return;

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
	SetPlayerInput(false);
}

void ATromboneCharacterBase::UnapplyStun()
{
	if (bIsRagdoll) return;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	SetPlayerInput(true);
}

void ATromboneCharacterBase::ApplyRagdoll()
{
	SetPlayerInput(false);

    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
    
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->SetIsRagdolling(true);
	}
}

void ATromboneCharacterBase::UnapplyRagdoll()
{
    const FVector PelvisLocation = GetMesh()->GetSocketLocation(PelvisBoneName);
    const FRotator PelvisRotation = GetMesh()->GetSocketRotation(PelvisBoneName);

    FVector TargetCapsuleLocation = PelvisLocation;
    const FRotator TargetCapsuleRotation = FRotator(0.0f, PelvisRotation.Yaw + 90.0f, 0.0f);

    const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    FHitResult HitResult;
    FVector Start = PelvisLocation;
    FVector End = PelvisLocation - FVector(0.0f, 0.0f, CapsuleHalfHeight * 2.0f);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
       TargetCapsuleLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, CapsuleHalfHeight + 2.0f);
    }

    SetActorLocationAndRotation(TargetCapsuleLocation, TargetCapsuleRotation);
    GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0.0f, -90.0f, 0.0f));

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandler_DelayedSavePostSnapshot, 
		this, 
		&ThisClass::DelayedSavePoseSnapshot, 
		PoseSnapshotInterval,
		false
	);
}

void ATromboneCharacterBase::DelayedSavePoseSnapshot()
{
	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->SaveRagdollPoseSnapshot();
	}

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandler_InternalUnapplyRagdoll, 
		this, 
		&ThisClass::InternalUnapplyRagdoll, 
		PoseSnapshotInterval,
		false
	);
}
void ATromboneCharacterBase::InternalUnapplyRagdoll()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	if (UCharacterAnimInstance* AnimInst = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->PlayGetUpMontage(IsFacingUp());
	}

	ApplyFlagPhysics();
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
	if (bIsRagdoll)
	{
		GetWorld()->GetTimerManager().ClearTimer(OnHitTimerHandle);
		EndRagdoll();
	}
	else
	{
		OnRagdoll();
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

bool ATromboneCharacterBase::IsFacingUp() const
{
	if (!GetMesh()) return true;

	const FRotator PelvisRotation = GetMesh()->GetSocketRotation(PelvisBoneName);
	const FVector PelvisUp = FRotationMatrix(PelvisRotation).GetScaledAxis(EAxis::Z);
    
	return (FVector::DotProduct(PelvisUp, FVector::UpVector) > 0.0f);
}

void ATromboneCharacterBase::ApplyFlagPhysics()
{
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;

	const UGameStateSubsystem* GameStateSubsystem = GI->GetSubsystem<UGameStateSubsystem>();
	if (!GameStateSubsystem || GameStateSubsystem->GetLevelState() != ELevelType::InGame) return;
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	FPhysicalAnimationData FlagAnimData;
	FlagAnimData.bIsLocalSimulation = false;
	FlagAnimData.OrientationStrength = 10.0f;
	FlagAnimData.AngularVelocityStrength = 5.0f;
	FlagAnimData.PositionStrength = 10.0f;
	FlagAnimData.VelocityStrength = 0.0f;
	FlagAnimData.MaxAngularForce = 0.0f;
	FlagAnimData.MaxLinearForce = 0.0f;

	FName BoneName = FName("flage01");
	GetMesh()->SetAllBodiesBelowSimulatePhysics(BoneName, true, true);
	PhysicalAnimationComp->ApplyPhysicalAnimationSettingsBelow(BoneName, FlagAnimData, true);
}

void ATromboneCharacterBase::OnRep_IsRagdoll()
{
	if (bIsRagdoll)
	{
		ApplyRagdoll();
		PlayFaceSequence(ECharacterFaceState::Ragdoll);
		if (AkSoundComponent && RagdollBooSound)
		{
			AkSoundComponent->PostAkEvent(RagdollBooSound, 0, FOnAkPostEventCallback());
		}
		OnRagdollDelegate.Broadcast();
	}
	else
	{
		UnapplyRagdoll();
		PlayFaceSequence(ECharacterFaceState::Blink);
		EndRagdollDelegate.Broadcast();
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