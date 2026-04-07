// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/RagdollComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


URagdollComponent::URagdollComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	traceHitCollision.Empty();
	//WorldStatic 추가
	traceHitCollision.Add(EObjectTypeQuery::ObjectTypeQuery1);
}

void URagdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (isWorldServer && isRagdoll && !isRagdollBaked)
	{
		if (isRagdollRecovering)
		{
			TickRagdollBlend();
			Multicast_Rotation();
		}
		else
		{
			TickRagdollBlend();
			serverRagdollPose = SnapshotServerBuild(TEXT("update_pose"));
			Multicast_Pose(serverRagdollPose.LocalTransforms);
			FVector PelvisLocation;
			FVector TraceHitLocation;
			bool TraceHitGround, PelvisDistanceGrounded;
			float PelvisDistanceToGround;
			TickCapsuleLocation(PelvisLocation, capsuleLocation, TraceHitLocation, TraceHitGround, isRagdollGrounded, PelvisDistanceGrounded, PelvisDistanceToGround);
			Multicast_Rotation();
			Multicast_Location();
			if (enableAutoRecover)
			{
				Server_Auto_Reset();
			}
		}
	}
}

void URagdollComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(URagdollComponent, characterReference);
	DOREPLIFETIME(URagdollComponent, isRagdoll);
	DOREPLIFETIME(URagdollComponent, isRagdollGrounded);
	DOREPLIFETIME(URagdollComponent, isRagdollRecovering);
	DOREPLIFETIME(URagdollComponent, isRagdollBaked);
	DOREPLIFETIME(URagdollComponent, isRagdollFatal);
	DOREPLIFETIME(URagdollComponent, isRagdollAnimated);
	DOREPLIFETIME(URagdollComponent, capsuleLocation);
}

void URagdollComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		if (GetOwner()->HasAuthority())
		{
			FTimerHandle BeginPlayHandle;
			World->GetTimerManager().SetTimer(BeginPlayHandle, [this]()
				{
					if (ACharacter* MyChar = Cast<ACharacter>(GetOwner()))
					{
						characterReference = MyChar;
					}
				}, 1.0f, false);
		}
	}
	
	
}

void URagdollComponent::OnRep_CharacterReference()
{
	if (characterReference)
	{
		skeletalMesh = characterReference->GetMesh();
		if (characterReference->IsLocallyControlled())
		{
			isWorldOwner = true;
		}
		if (characterReference->HasAuthority())
		{
			isWorldServer = true;
		}
		if (isWorldServer)
		{
			// 캐릭터가 서버 화면(카메라)에 안 보여도 항상 애니메이션 포즈 계산 및 본 업데이트 수행
			skeletalMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
			// 래그돌과 캐릭터의 업데이트 주기 설정
			characterReference->SetNetUpdateFrequency(netMaxFPS);
			characterReference->SetMinNetUpdateFrequency(netMinFPS);
			Server_FindPelvisOffset();
			Server_RefreshRagdolls();
		}
	}
}



void URagdollComponent::Client_RefreshRagdolls_Implementation()
{
	if (isRagdoll)
	{
		//현재 래그돌이 bake되면 새로 들어온 클라이언트에게 Bake된 상태를 알려줘야함.
		if (isRagdollBaked)
		{
			Client_Ragdoll_Bake();
		}
		else
		{
			//현재 래그돌이 Recovering상태이면 새로 들어온 클라이언트에게 정보 전달
			if (isRagdollRecovering)
			{
				Client_Ragdoll_Recover(false);
			}
			else
			{
				Client_Ragdoll_Start(isRagdollFatal);
			}
		}
	}
}

void URagdollComponent::Client_Ragdoll_Bake()
{
	RPC_Ragdoll_Bake();
}

void URagdollComponent::RPC_Ragdoll_Bake_Implementation()
{
	if (!CheckReference()) return;
	// 서버에서 현재 캐릭터를 검사했을때 이미 Bake되어있으면
	// 새로 들어온 플레이어를 포함해 모두에게 결과값만 업데이트
	if (isRagdollBaked)
	{
		TArray<FTransform> OutTransforms = serverBakedPose.LocalTransforms;
		Multicast_Ragdoll_Bake(OutTransforms);
	}
	else
	{
		isRagdoll = true;
		isRagdollBaked = true;
		isRagdollRecovering = false;
		ragdollBlend = 0.f;
		ragdollResetTimer = 0.f;
		serverBakedPose = SnapshotServerBuild(TEXT("baked_pose"));
		skeletalMesh->SetSimulatePhysics(false);
		Multicast_Ragdoll_Bake(serverBakedPose.LocalTransforms);
	}
}

void URagdollComponent::Multicast_Ragdoll_Bake_Implementation(const TArray<FTransform>& InBonesTransform)
{
	if (!CheckReference()) return;
	clientBakedPose = SnapshotClientBuild(InBonesTransform);

	skeletalMesh->GetAnimInstance()->Montage_Stop(0.f);
	characterReference->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	characterReference->GetCharacterMovement()->SetMovementMode(MOVE_None);
}

void URagdollComponent::Client_Ragdoll_Recover(bool InDisableNoAnim)
{
	int32 PoseID = 0;
	float PelvisYaw = 0.f;
	GetPelvisRotation(PoseID, PelvisYaw);
	RPC_Ragdoll_Recover(InDisableNoAnim, PoseID, PelvisYaw);
}

void URagdollComponent::Client_Ragdoll_Start(bool InIsFatal)
{
	RPC_Ragdoll_Start(InIsFatal);
}

void URagdollComponent::Multicast_Rotation_Implementation()
{
	TickUpdateRotation();
}
void URagdollComponent::Multicast_Location_Implementation()
{
	TickUpdateLocation();
}

void URagdollComponent::Multicast_Pose_Implementation(const TArray<FTransform>& InBonesTransform)
{
	clientRagdollPose = SnapshotClientBuild(InBonesTransform, TEXT("update_pose"));
}


void URagdollComponent::Multicast_Ragdoll_Recover_End_Implementation()
{
	skeletalMesh->GetAnimInstance()->Montage_Stop(0.25f);
}


void URagdollComponent::RPC_Ragdoll_Start_Implementation(bool InIsFatal)
{
	if (!CheckReference()) return;
	Multicast_Ragdoll_Start(InIsFatal, GetCharacterVelocity(false));
	Client_Ragdoll_Notify(true, InIsFatal, false, false);
	Server_Ragdoll_Notify(true, InIsFatal, false, false);
}

void URagdollComponent::Multicast_Ragdoll_Start_Implementation(bool InIsFatal, const FVector& InInitVelocity)
{
	if (!CheckReference()) return;
	InitRagdoll(InIsFatal, InInitVelocity);
}

void URagdollComponent::Client_Ragdoll_Notify_Implementation(bool InInit, bool InIsFatal,
	bool InRecoverStart, bool InRecoverEnd)
{
	OnRagdollUpdatedClient.Broadcast(InInit, InIsFatal, InRecoverEnd, InRecoverEnd);
}

void URagdollComponent::Server_Ragdoll_Notify_Implementation(bool InInit, bool InIsFatal,
	bool InRecoverStart, bool InRecoverEnd)
{
	OnRagdollUpdatedServer.Broadcast(InInit, InIsFatal, InRecoverStart, InRecoverEnd);
}

void URagdollComponent::Multicast_Ragdoll_Recover_Start_Implementation(bool InDisableNoAnim,
                                                                       int32 InPoseId, float InPoseYaw, const FVector& InRecoverVelocity)
{
	if (!CheckReference()) return;
	float ResetTime = RecoverRagdoll(InDisableNoAnim, InPoseId, InPoseYaw, InRecoverVelocity);
	if (isWorldServer)
	{
		ragdollResetTimer = ResetTime;
		if (ragdollResetTimer == 0.f)
		{
			OnRagdollResetTimerCompleted();
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(
				RetriggerableDelayHandle,
				this,
				&ThisClass::OnRagdollResetTimerCompleted,
				ragdollResetTimer,
				false
			);
		}
	}

}

void URagdollComponent::RPC_Ragdoll_Recover_Implementation(bool InDisableNoAnim, int32 InPoseId, float InPoseYaw)
{
	int32 PoseID = 0;
	float PelvisYaw = 0.f;
	GetPelvisRotation(PoseID, PelvisYaw);

	if (clientSidedPoseCheck)
	{
		PoseID = InPoseId;
		PelvisYaw = InPoseYaw;
	}
	FVector RecoverVelocity = GetCharacterVelocity(true);
	Multicast_Ragdoll_Recover_Start(InDisableNoAnim, PoseID, InPoseYaw, RecoverVelocity);
	Client_Ragdoll_Notify(false,false,true,false);
	Server_Ragdoll_Notify(false,false,true,false);
}

void URagdollComponent::Server_FindPelvisOffset()
{
	// Ragdoll의 Capsule Location을 찾기 위해 수행
	if (!isPelvisFound)
	{
		skeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode, true);
		//빈 애니메이션 사용해서 rest pose로 만듬
		skeletalMesh->SetAnimation(nullptr);
		pelvisOffset = characterReference->GetActorLocation() - skeletalMesh->GetSocketLocation(pelvisName);
		//다시 animBP 모드로 변환
		skeletalMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint, true);
		isPelvisFound = true;
	}
}

void URagdollComponent::Server_RefreshRagdolls()
{
	// 누군가가 게임에 접속했을때 해당 플레이어가 다른 플레이어들의 래그돌 상태를 업데이트받아야함.
	for (TObjectIterator<URagdollComponent> It; It; ++It)
	{
		//현재 월드에 속한 컴포넌트가 아니면 건너뜀
		if (It->GetWorld() != GetWorld())
		{
			continue;
		}

		if (IsValid(*It))
		{
			It->Client_RefreshRagdolls();
		}
	}
}

void URagdollComponent::TickRagdollBlend()
{
	if (IsValid(skeletalMesh))
	{
		if (isRagdollRecovering)
		{
			ragdollBlend = FMath::Clamp(FMath::FInterpTo(ragdollBlend, -0.05, GetWorld()->GetDeltaSeconds(), ragdollBlendOut),0.f,1.f);
		}
		else
		{
			ragdollBlend = FMath::Clamp(FMath::FInterpTo(ragdollBlend, 1.05, GetWorld()->GetDeltaSeconds(), ragdollBlendIn), 0.f, 1.f);
		}

		skeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(pelvisName, ragdollBlend, false, true);
		if (isRagdollRecovering)
		{
			if (ragdollBlend == 0.f)
			{
				if (skeletalMesh->IsAnySimulatingPhysics())
				{
					skeletalMesh->SetSimulatePhysics(false);
				}
				if (ragdollResetTimer == -1.0f)
				{
					ragdollResetTimer = 0.f;
					isRagdollRecovering = false;
					isRagdoll = false;
					Multicast_Ragdoll_Recover_End();
					Client_Ragdoll_Notify(false, false, false, true);
					Server_Ragdoll_Notify(false, false, false, true);
				}
			}
		}
		else
		{
			ragdollResetTimer = 0.f;
		}

	}
}

void URagdollComponent::TickUpdateRotation()
{
	//lerp ragdoll rotation, the only problem with snapshoted ragdoll sync, is blend back between snapshoted pose rotation and character rotation
	//while animation montage starts playing, or whatever(blend goes back from snapshot to default anim state) the character will reset its rotation
	//the problem is while in snapshoted pose it does rotate character's mesh instead of character's root, that's why i found this way to lerp between
	//character's pelvis yaw, this compensates mesh rotation reset so it does look smooth enough.

	UWorld* World = GetWorld();
	if (!CheckReference() || !World) return;
	
	if (isRagdollRecovering)
	{
		recoverAlpha = FMath::Clamp(recoverAlpha + recoverAlpha + World->GetDeltaSeconds(), 0.f, 0.99f);
		FRotator Rot = characterReference->GetActorRotation();
		Rot.Yaw = recoverYaw;
		characterReference->SetActorRotation(UKismetMathLibrary::RLerp(characterReference->GetActorRotation(), Rot, recoverAlpha, true));
	}
	else
	{
		recoverAlpha = 0.f;
	}
}

void URagdollComponent::TickUpdateLocation()
{
	//ragdoll update location for multicast, smooth update, editable interp speed
	UWorld* World = GetWorld();
	if (!CheckReference() || !World) return;
	FVector NewLocation = UKismetMathLibrary::VInterpTo(characterReference->GetActorLocation(), capsuleLocation, World->GetDeltaSeconds(), capsuleInterp);
	characterReference->SetActorLocation(NewLocation);
}

void URagdollComponent::TickCapsuleLocation(FVector& OutPelvisLocation, FVector& OutCapsuleLocation,
                                            FVector& OutTraceHitLocation, bool& OutTraceHitGround, bool& OutPelvisFullyGrounded, bool& OutPelvisDistanceGrounded,
                                            float& OutPelvisDistanceToGround)
{
	FVector LocalPelvisLoc = skeletalMesh->GetSocketLocation(pelvisName);
	float LocalCapsuleRadius;
	float LocalCapsuleHeight;
	bool LocalHitGround;
	FVector LocalHitLoc;
	FVector LocalCapsuleLoc;
	characterReference->GetCapsuleComponent()->GetScaledCapsuleSize_WithoutHemisphere(LocalCapsuleRadius, LocalCapsuleHeight);

	FVector Start = LocalPelvisLoc + pelvisOffset + characterReference->GetActorUpVector() * LocalCapsuleHeight * 1.2f;
	FVector End = LocalPelvisLoc + pelvisOffset + characterReference->GetActorUpVector() * LocalCapsuleHeight * 1.2f * -1.f;
	TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	LocalHitGround =  UKismetSystemLibrary::SphereTraceSingleForObjects(
		this, Start, End,
		LocalCapsuleRadius,
		traceHitCollision,
		false,
		ActorsToIgnore,
		DebugTraceMode,
		HitResult,
		true);

	if (LocalHitGround)
	{
		LocalHitLoc = HitResult.Location;
		//Hit결과 true일 경우, Capsule Trace Location을 씀
		LocalCapsuleLoc = LocalHitLoc + FVector(0.f,0.f,LocalCapsuleHeight);
	}
	else
	{
		LocalHitLoc = HitResult.TraceEnd;
		//Ragdoll이 공중에 있을 경우, Pelvis를 Capsule의 Root location으로 사용
		LocalCapsuleLoc = LocalPelvisLoc;
	}

	float LocalPelvisDistanceToGround = UKismetMathLibrary::VSize(LocalPelvisLoc + pelvisOffset - LocalHitLoc);
	bool LocalPelvisFullyGrounded = LocalPelvisDistanceToGround <= pelvisGroundDistance && LocalHitGround;
	bool LocalPelvisDistanceGrounded = LocalPelvisDistanceToGround <= pelvisGroundDistance;
	

	OutPelvisLocation = LocalPelvisLoc;
	OutCapsuleLocation = LocalCapsuleLoc;
	OutTraceHitLocation = LocalHitLoc;
	OutTraceHitGround = LocalHitGround;
	OutPelvisFullyGrounded = LocalPelvisFullyGrounded;
	OutPelvisDistanceGrounded = LocalPelvisDistanceGrounded;
	OutPelvisDistanceToGround = LocalPelvisDistanceToGround;
}

void URagdollComponent::Server_Auto_Reset()
{
	//tick auto reset timer
	//to have automatic get-up if body is calm and not fatal

	//Ragdoll auto reseting sequence, should be called only from server
	if (!isRagdollRecovering && !isRagdollFatal)
	{
		if (isRagdollGrounded)
		{
			FVector LocalPhysicsLinearVelocity = skeletalMesh->GetPhysicsLinearVelocity(pelvisName);
			if (UKismetMathLibrary::VSize(LocalPhysicsLinearVelocity) <= autoRecoverVelocity)
			{
				serverRecoverTimer = serverRecoverTimer + GetWorld()->GetDeltaSeconds();
				if (serverRecoverTimer >= autoRecoverTime)
				{
					serverRecoverTimer = 0.f;
					Client_Ragdoll_Recover(false);
				}
			}
			else
			{
				//몸이 매우 빠르게 움직이고 있거나,
				//Pelvis가 Grounded상태가 아닐 경우 타이머 리셋
				serverRecoverTimer = 0.f;
			}
		}
		else
		{
			serverRecoverTimer = 0.f;
		}
	}
}


FPoseSnapshot URagdollComponent::SnapshotClientBuild(const TArray<FTransform>& InBonesTransform, const FName& InName)
{
	//본의 부모를 기준으로 한 상대적인 위치/회전값을 snapshot으로 저장
	FPoseSnapshot ResultSnapShot;
	// 초기값 설정
	ResultSnapShot.bIsValid = false;

	// 메쉬와 참조가 모두 유효할 때만 로직 실행
	if (CheckReference() && skeletalMesh)
	{
		ResultSnapShot.LocalTransforms = InBonesTransform;
		ResultSnapShot.BoneNames = skeletalMesh->GetAllSocketNames();
		ResultSnapShot.SkeletalMeshName = FName(*skeletalMesh->GetSkeletalMeshAsset()->GetName());
		ResultSnapShot.SnapshotName = InName;
		ResultSnapShot.bIsValid = true;
	}

	return ResultSnapShot;
}

FPoseSnapshot URagdollComponent::SnapshotServerBuild(const FName& InName)
{
	//본의 부모를 기준으로 한 상대적인 위치/회전값을 snapshot으로 저장
	FPoseSnapshot ResultSnapShot;
	TArray<FTransform> LocalBones;
	if (CheckReference())
	{
		TArray<FName> SocketNames = skeletalMesh->GetAllSocketNames();
		for (int32 i = 0;i<SocketNames.Num();++i)
		{
			if (IsValid(skeletalMesh))
			{
				FTransform BoneTransform = skeletalMesh->GetSocketTransform(SocketNames[i], RTS_ParentBoneSpace);
				LocalBones.Add(BoneTransform);
			}
		}
	}
	ResultSnapShot.LocalTransforms = LocalBones;
	ResultSnapShot.BoneNames = skeletalMesh->GetAllSocketNames();
	ResultSnapShot.SnapshotName = InName;
	ResultSnapShot.bIsValid = true;
	return ResultSnapShot;
}

void URagdollComponent::InitRagdoll(const bool& InIsFatal, const FVector& InInitVelocity)
{
	isRagdollFatal = InIsFatal;
	FVector LocalVelocity = InInitVelocity;
	capsuleLocation = characterReference->GetActorLocation();

	if (isRagdollFatal)
	{
		isRagdollAnimated = false;
		skeletalMesh->bUpdateJointsFromAnimation = isRagdollAnimated;
		skeletalMesh->SetAllMotorsAngularDriveParams(0.f, 0.f, 0.f, false);
	}
	else
	{
		isRagdollAnimated = true;
		skeletalMesh->bUpdateJointsFromAnimation = isRagdollAnimated;
		skeletalMesh->SetAllMotorsAngularDriveParams(animationSpringPower, 0.f, 0.f, false);
	}

	skeletalMesh->GetAnimInstance()->Montage_Stop(0.1f);
	characterReference->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	characterReference->GetCharacterMovement()->SetMovementMode(MOVE_None);

	if (isWorldServer)
	{
		skeletalMesh->SetSimulatePhysics(true);
		if (isRagdollRecovering)
		{
			ragdollBlend = 1.0f;
			skeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(pelvisName, ragdollBlend, false, true);
		}
		skeletalMesh->SetAllPhysicsLinearVelocity(LocalVelocity, false);
	}
	else
	{
		skeletalMesh->SetSimulatePhysics(false);
	}

	isRagdollRecovering = false;
	isRagdollBaked = false;
	isRagdoll = true;
}


float URagdollComponent::RecoverRagdoll(const bool& InDisableNoAnim, const int32& InPose, const float& InYaw,
	const FVector& InRecoverVelocity)
{
	bool LocalDisableNoAnim = InDisableNoAnim;
	int32 LocalPoseID = InPose;
	recoverYaw = InYaw;
	FVector LocalVelocity = InRecoverVelocity;
	float LocalResetTime = 0.f;
	UAnimMontage* LocalMontage;
	float LocalApex = 0.f;

	if (isRagdoll)
	{
		isRagdollRecovering = true;
		isRagdollBaked = false;
		serverRecoverTimer = 0.f;
		skeletalMesh->GetAnimInstance()->SavePoseSnapshot(TEXT("ragdoll_recover"));
		skeletalMesh->GetAnimInstance()->Montage_Stop(0.f);
		characterReference->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 포즈가 공중일 경우 fall 사용. 아니면 Walk 사용
		if (LocalPoseID == 2)
		{
			characterReference->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
		else
		{
			characterReference->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		characterReference->LaunchCharacter(LocalVelocity, true, true);

		if (LocalDisableNoAnim)
		{
			return LocalResetTime;
		}
		else
		{
			LocalMontage = (LocalPoseID == 0) ? recoverBackwardMontage : recoverForwardMontage;
			LocalApex = (LocalPoseID == 0) ? recoverBackwardApex : recoverForwardApex;
			currentAnimDuration = skeletalMesh->GetAnimInstance()->Montage_Play(LocalMontage, 1.f, EMontagePlayReturnType::Duration, 0.f, true);

			if (LocalPoseID == 2 || !isRagdollGrounded)
			{
				LocalResetTime = 0.f;
			}
			else
			{
				LocalResetTime = currentAnimDuration * LocalApex;
			}
			return LocalResetTime;
		}
	}
	return LocalResetTime;
}

bool URagdollComponent::CheckReference()
{
	return IsValid(characterReference) && IsValid(skeletalMesh);
}

void URagdollComponent::GetPelvisRotation(int32& OutPoseId, float& OutPelvisYaw)
{
	if (!CheckReference()) return;

	int32 LocalPose = 0;
	float PelvisYaw = 0.f;

	FRotator Rot = skeletalMesh->GetSocketRotation(pelvisName);

	if (!isRagdollGrounded)
	{
		LocalPose = 2;
	}
	else
	{
		if (Rot.Roll > 0)
		{
			LocalPose = 1;
		}
		else
		{
			LocalPose = 0;
		}
	}

	if (Rot.Roll > 0)
	{
		PelvisYaw = Rot.Yaw;
	}
	else
	{
		PelvisYaw = Rot.Yaw - 180.f;
	}
	OutPoseId = LocalPose;
	OutPelvisYaw = PelvisYaw;
}

FVector URagdollComponent::GetCharacterVelocity(bool InUseRagdollVelocity)
{
	//Physics 적용 후 Mesh에 적용할 Velocity를 계산
	//Physics 사용 안할 시 Mesh Velocity 사용
	FVector ResultVector;
	if (!CheckReference()) return ResultVector;

	if (InUseRagdollVelocity)
	{
		ResultVector = skeletalMesh->GetPhysicsLinearVelocity(pelvisName);
	}
	else
	{
		ResultVector = characterReference->GetVelocity();
	}
	return ResultVector;
}

void URagdollComponent::OnRagdollResetTimerCompleted()
{
	if (isRagdollRecovering)
	{
		ragdollResetTimer = -1.0f;
	}
}



