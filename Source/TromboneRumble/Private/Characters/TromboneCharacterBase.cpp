// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Characters/TromboneCharacterBase.h"
#include "AkComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Utilities/DebugHelper.h"

namespace
{
	constexpr float FallbackStunDuration = 2.5f;
	constexpr float FallbackInvincibilityDuration = 1.0f;
}

ATromboneCharacterBase::ATromboneCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

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

void ATromboneCharacterBase::AddBlock(const ECharacterBlockReason Reason)
{
	const uint8 OldMask = BlockMask;
	BlockMask |= static_cast<uint8>(Reason);

	// Only the first reason notifies. Later ones just stack onto the mask
	if (OldMask == 0 && BlockMask != 0)
	{
		OnBlockedStateChanged(true);
	}
}

void ATromboneCharacterBase::RemoveBlock(const ECharacterBlockReason Reason)
{
	const uint8 OldMask = BlockMask;
	BlockMask &= ~static_cast<uint8>(Reason);

	// Only the last reason leaving notifies
	if (OldMask != 0 && BlockMask == 0)
	{
		OnBlockedStateChanged(false);
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
		RemoveBlock(ECharacterBlockReason::ServerLock);
	}
	else
	{
		AddBlock(ECharacterBlockReason::ServerLock);
	}
}

void ATromboneCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (PhysicalAnimationComp)
	{
		PhysicalAnimationComp->SetSkeletalMeshComponent(GetMesh());
	}

	// 래그돌 상태 처리 바인딩. 연출 핸들러는 파생이 같은 델리게이트에 별도로 바인딩한다
	if (RagdollComponent)
	{
		RagdollComponent->OnRagdollStarted.AddDynamic(this, &ThisClass::HandleRagdollStarted);
		RagdollComponent->OnRagdollEnded.AddDynamic(this, &ThisClass::HandleRagdollEnded);
	}

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
}

void ATromboneCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ATromboneCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bInputEnabled);
	DOREPLIFETIME(ThisClass, bIsStun);
	DOREPLIFETIME(ThisClass, bIsInvincible);
}

bool ATromboneCharacterBase::OnHitReceived_Implementation(const FHitData& HitData)
{
	if (!HasAuthority()) return false;

	if (!CanReceiveHit()) return false;

	const FVector KnockbackVel = CalculateKnockbackVelocity(HitData);

	switch (HitData.HitReaction)
	{
		case EHitReactionType::Ragdoll:
			if (RagdollComponent)
			{
				RagdollComponent->StartRagdoll(KnockbackVel, CalculateKnockbackSpin(KnockbackVel));
			}
			break;

		case EHitReactionType::Stun:
			OnStun();
			LaunchCharacter(KnockbackVel, true, true);
			// AI 폰은 소유 클라가 없어 Client RPC를 보낼 수 없다 (No owning connection 경고 방지)
			if (IsPlayerControlled())
			{
				Client_ApplyKnockback(KnockbackVel);
			}
			break;

		case EHitReactionType::KnockbackOnly:
			LaunchCharacter(KnockbackVel, true, true);
			if (IsPlayerControlled())
			{
				Client_ApplyKnockback(KnockbackVel);
			}
			break;

		case EHitReactionType::None:
			; // intentional fall through

		default:
			break;
	}

	return true;
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

FVector ATromboneCharacterBase::CalculateKnockbackSpin(const FVector& KnockbackVelocity) const
{
	const float SpinRate = CharacterData ? CharacterData->KnockbackSpinRate : 0.f;
	if (SpinRate <= 0.f)
	{
		return FVector::ZeroVector;
	}

	// 진행 방향과 위쪽의 외적 = 옆으로 누운 축. 그 축으로 돌면 밀려나는 쪽으로 굴러간다
	const FVector SpinAxis = FVector::CrossProduct(KnockbackVelocity.GetSafeNormal2D(), FVector::UpVector);
	return SpinAxis * SpinRate;
}

void ATromboneCharacterBase::Client_ApplyKnockback_Implementation(const FVector KnockbackVelocity)
{
	if (HasAuthority())
	{
		return;
	}

	LaunchCharacter(KnockbackVelocity, true, true);
}

bool ATromboneCharacterBase::IsRagdoll() const
{
	return RagdollComponent ? RagdollComponent->IsRagdoll() : false;
}

FVector ATromboneCharacterBase::GetPelvisLocation() const
{
	const USkeletalMeshComponent* MeshComp = GetMesh();
	return MeshComp ? MeshComp->GetSocketLocation(TromboneBones::Pelvis) : GetActorLocation();
}

void ATromboneCharacterBase::OnStun()
{
	if (!HasAuthority()) return;

	if (IsRagdoll() || bIsStun) return;

	bIsStun = true;
	OnRep_IsStun();

	const float StunDuration = CharacterData ? CharacterData->StunDuration : FallbackStunDuration;
	GetWorld()->GetTimerManager().SetTimer(
		OnHitTimerHandle,
		this,
		&ThisClass::EndStun,
		StunDuration,
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

	const float InvincibleDuration = CharacterData ? CharacterData->InvincibilityDurationAfterStun : FallbackInvincibilityDuration;
	GetWorld()->GetTimerManager().SetTimer(
		InvincibilityTimerHandle,
		[this]()
		{
			bIsInvincible = false;
			OnRep_IsInvincible();
		},
		InvincibleDuration,
		false
	);
}

void ATromboneCharacterBase::ApplyStun()
{
	StopAnimMontage();
	AddBlock(ECharacterBlockReason::Stun);
}

void ATromboneCharacterBase::UnapplyStun()
{
	RemoveBlock(ECharacterBlockReason::Stun);

	if (IsRagdoll())
	{
		return;
	}

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
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
	AddBlock(ECharacterBlockReason::Ragdoll);

	bool bIsLobby = false;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UGameStateSubsystem* GameStateSubsystem = GI->GetSubsystem<UGameStateSubsystem>())
		{
			bIsLobby = IsLobbyLevelType(GameStateSubsystem->GetLevelState());
		}
	}
	
	if (!bIsLobby && AkSoundComponent && RagdollBooSound)
	{
		AkSoundComponent->PostAkEvent(RagdollBooSound, 0, FOnAkPostEventCallback());
	}
}

void ATromboneCharacterBase::Multicast_PlayFallScream_Implementation()
{
	if (AkSoundComponent && FallScreamSound)
	{
		AkSoundComponent->PostAkEvent(FallScreamSound, 0, FOnAkPostEventCallback());
	}
}

void ATromboneCharacterBase::Multicast_PlayLandPain_Implementation()
{
	if (AkSoundComponent && LandPainSound)
	{
		AkSoundComponent->PostAkEvent(LandPainSound, 0, FOnAkPostEventCallback());
	}
}

void ATromboneCharacterBase::SetLandingSoundEnabled(const bool bEnable)
{
	if (!HasAuthority() || !GetMesh())
	{
		return;
	}

	GetMesh()->SetAllBodiesNotifyRigidBodyCollision(bEnable);

	GetMesh()->OnComponentHit.RemoveDynamic(this, &ThisClass::HandleRagdollLandingHit);
	if (bEnable)
	{
		GetMesh()->OnComponentHit.AddDynamic(this, &ThisClass::HandleRagdollLandingHit);
	}
}

void ATromboneCharacterBase::HandleRagdollLandingHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (NormalImpulse.SizeSquared() < FMath::Square(LandingImpulseThreshold))
	{
		return;
	}

	SetLandingSoundEnabled(false);
	Multicast_PlayLandPain();
}

void ATromboneCharacterBase::HandleRagdollEnded()
{
	if (HasAuthority())
	{
		bIsInvincible = true;
		OnRep_IsInvincible();

		const float InvincibleDuration = CharacterData ? CharacterData->InvincibilityDurationAfterRagdoll : FallbackInvincibilityDuration;
		GetWorld()->GetTimerManager().SetTimer(
			InvincibilityTimerHandle,
			[this]()
			{
				bIsInvincible = false;
				OnRep_IsInvincible();
			},
			InvincibleDuration,
			false
		);
	}
}

void ATromboneCharacterBase::OnRep_IsStun()
{
	if (bIsStun)
	{
		ApplyStun();

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

		if (StunNiagaraComponent)
		{
			StunNiagaraComponent->DeactivateImmediate();
		}
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
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