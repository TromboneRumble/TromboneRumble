// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/Present/Present.h"
#include "Components/SphereComponent.h"
#include "AkComponent.h"
#include "AkGameplayTypes.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Framework/DefaultPlayerState.h"
#include "Utilities/Defines.h"

APresent::APresent()
{
	PrimaryActorTick.bCanEverTick = true;

	// GiftMesh를 루트로 사용해야 SetActorLocation sweep으로 지면 충돌 감지 가능
	GiftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GiftMesh"));
	SetRootComponent(GiftMesh);
	GiftMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GiftMesh->SetCollisionObjectType(ECC_WorldDynamic);
	GiftMesh->SetCollisionResponseToAllChannels(ECR_Block);
	GiftMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(GiftMesh);
	OverlapSphere->SetSphereRadius(80.f);
	OverlapSphere->SetCollisionProfileName(TEXT("Trigger"));
	
	PendulumPivot = CreateDefaultSubobject<USceneComponent>(TEXT("PendulumPivot"));
	PendulumPivot->SetupAttachment(GiftMesh);
	PendulumPivot->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	
	BalloonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BalloonMesh"));
	BalloonMesh->SetupAttachment(PendulumPivot);
	BalloonMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	if (AkComponent)
	{
		AkComponent->OcclusionRefreshInterval = 0.f;
		AkComponent->SetupAttachment(RootComponent);
	}

	bReplicates = true;
	SetReplicateMovement(false);
}

void APresent::BeginPlay()
{
	Super::BeginPlay();
	PendulumLength = PendulumPivot->GetRelativeLocation().Z;
	if (HasAuthority())
	{
		// 서버에서 운동 상수 결정 및 복제
		AnchorSpawnLocation = GetActorLocation() + FVector(0.f, 0.f, PendulumLength);
		ReplicatedSwayPhaseOffset = FMath::RandRange(0.f, 2.f * PI);
		
		const float RandomAngle = FMath::RandRange(0.f, 2.f * PI);
		ReplicatedSwayAxis = FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.f);
	}
	else
	{
		// 클라이언트는 지면과의 직접 충돌을 무시 (서버의 OnLanded 동기화에 의존)
		GiftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APresent::HandleOverlapBegin);
	SetActorTickEnabled(true);
}

void APresent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bHasLanded) return;

	SwayTime += DeltaTime;

	//수직 위치 계산
	const float FallDistance = InitialFallSpeed * SwayTime + 0.5f * GravityAccel * SwayTime * SwayTime;
	FVector CurrentAnchorLocation = AnchorSpawnLocation;
	CurrentAnchorLocation.Z -= FallDistance;
	
	//진자 회전 계산
	const float SineValue = FMath::Sin(2.f * PI * SwayFrequency * SwayTime + ReplicatedSwayPhaseOffset);
    const float CurrentAngle = MaxPendulumAngleDeg * SineValue;
    FVector RotationAxis = FVector::CrossProduct(ReplicatedSwayAxis, FVector::UpVector);
    FQuat PendulumQuat = FQuat(RotationAxis, FMath::DegreesToRadians(CurrentAngle));
	
	FVector OffsetFromAnchor = PendulumQuat.RotateVector(FVector(0.f, 0.f, -PendulumLength));
	FVector TargetLocation = CurrentAnchorLocation + OffsetFromAnchor;
	
	if (HasAuthority())
	{
		// 서버에서 충돌(Sweep)을 수행
		FHitResult HitResult;
		SetActorLocationAndRotation(TargetLocation, PendulumQuat, true, &HitResult);
		
		if (HitResult.bBlockingHit)
		{
			bHasLanded = true;
			LandedLocation = GetActorLocation();
			LandedRotation = GetActorRotation();

			SetActorTickEnabled(false);
			PendulumPivot->SetRelativeRotation(FRotator::ZeroRotator);
			
			// 지면 법선에 맞춘 최종 회전값 설정
			const FRotator SurfaceRotation = FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).Rotator();
			LandedRotation = SurfaceRotation;
			SetActorRotation(LandedRotation);

			Multicast_HideBalloon();
		}
	}
	else
	{
		SetActorLocationAndRotation(TargetLocation, PendulumQuat, false);
	}
}

void APresent::OnRep_HasLanded()
{
	if (bHasLanded)
	{
		// 서버가 결정한 최종 위치와 회전으로 스냅(Snap)
		SetActorLocationAndRotation(LandedLocation, LandedRotation);
		PendulumPivot->SetRelativeRotation(FRotator::ZeroRotator);
		
		Multicast_HideBalloon();
	}
}

void APresent::Multicast_HideBalloon_Implementation()
{
	if (BalloonMesh)
	{
		BalloonMesh->SetVisibility(false, true);
	}
}

void APresent::HandleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bBonusAwarded) return;

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());

	if (PC && PC->IsLocalController())
	{
		if (AkComponent && PresentHitSoundEvent)
		{
			AkComponent->PostAkEvent(PresentHitSoundEvent, 0, FOnAkPostEventCallback());
		}

		// 로컬 스코어 UI 즉시 반영 (Trombone Rumble 프로젝트 구조 반영)
		if (ADefaultPlayerState* PS = Character->GetPlayerState<ADefaultPlayerState>())
		{
			PS->AddScore(BonusScore, EScoreType::Present);
		}
	}

	bBonusAwarded = true;

	if (HasAuthority())
	{
		Destroy();
	}
	else
	{
		Server_OnPlayerTouched();
	}
}

void APresent::Server_OnPlayerTouched_Implementation()
{
	if (IsValid(this)) Destroy();
}

void APresent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APresent, bHasLanded);
	DOREPLIFETIME(APresent, LandedLocation);
	DOREPLIFETIME(APresent, LandedRotation);
	DOREPLIFETIME(APresent, AnchorSpawnLocation);
	DOREPLIFETIME(APresent, ReplicatedSwayAxis);
	DOREPLIFETIME(APresent, ReplicatedSwayPhaseOffset);
}
