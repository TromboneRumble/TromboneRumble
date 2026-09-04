// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/Present/Present.h"
#include "Components/SphereComponent.h"
#include "AkGameplayTypes.h"
#include "AkGameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Framework/DefaultPlayerState.h"
#include "Utilities/Defines.h"

namespace
{
	/**
	 * 여러 옥타브를 겹쳐 정확히 반복되지 않는 파형. 반환 범위 -1 ~ 1.
	 * 배율이 무리수에 가까워 최소공배수가 사실상 없으므로 준주기적으로 보인다.
	 */
	float SampleWave(float Phase)
	{
		float Value = FMath::Sin(Phase);
		Value += 0.5f * FMath::Sin(Phase * 1.7f + 1.3f);
		Value += 0.25f * FMath::Sin(Phase * 2.9f + 2.7f);
		return Value / 1.75f;   // 진폭 합으로 나눠 -1~1로 정규화
	}

	/** 마루는 뾰족하고 골은 넓고 평평한, 실제 파도에 가까운 세로 변위. 반환 -1 ~ 1 */
	float SampleWaveCrest(float Phase, float Sharpness)
	{
		// 정규화 결과가 부동소수 오차로 0 미만이 될 수 있고, Pow에 음수가 들어가면 NaN이 된다
		const float Normalized = FMath::Clamp((SampleWave(Phase) + 1.f) * 0.5f, 0.f, 1.f);
		return FMath::Pow(Normalized, Sharpness) * 2.f - 1.f;
	}
}

APresent::APresent()
{
	PrimaryActorTick.bCanEverTick = true;

	// GiftMesh를 루트로 사용해야 SetActorLocation sweep으로 지면 충돌 감지 가능
	GiftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GiftMesh"));
	SetRootComponent(GiftMesh);
	GiftMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GiftMesh->SetCollisionObjectType(ECC_WorldDynamic);
	GiftMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(GiftMesh);
	OverlapSphere->SetSphereRadius(80.f);
	OverlapSphere->SetCollisionProfileName(TEXT("Trigger"));
	// 루트(GiftMesh)를 스케일해서 팝인을 하므로, 절대 스케일로 두지 않으면 획득 판정 반경까지 0배가 된다
	OverlapSphere->SetUsingAbsoluteScale(true);

	PendulumPivot = CreateDefaultSubobject<USceneComponent>(TEXT("PendulumPivot"));
	PendulumPivot->SetupAttachment(GiftMesh);
	PendulumPivot->SetRelativeLocation(FVector(0.f, 0.f, 80.f));

	BalloonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BalloonMesh"));
	BalloonMesh->SetupAttachment(PendulumPivot);
	BalloonMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	SparkleVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SparkleVFX"));
	SparkleVFX->SetupAttachment(GiftMesh);
	SparkleVFX->SetAutoActivate(false);

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.f, 90.f, 0.f);
	// 델타를 월드 공간에서 앞곱셈 → 기울어진 자세를 유지한 채 월드 Z축으로 돈다
	RotatingMovement->bRotationInLocalSpace = false;
	// 낙하 중 Tick의 진자 운동과 싸우지 않도록 착지 전까지 꺼둔다
	RotatingMovement->SetAutoActivate(false);

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

	if (bHasLanded)
	{
		TickLandedVisual(DeltaTime);
		return;
	}

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

			// 스윕이 멈춘 위치는 이미 메시 두께만큼 지면에서 떨어져 있으므로,
			// 여기에 HoverHeight만 더하면 메시 크기와 무관하게 일정하게 뜬다
			HoverLocation = GetActorLocation() + FVector(0.f, 0.f, HoverHeight);

			PendulumPivot->SetRelativeRotation(FRotator::ZeroRotator);

			EnterLandedState();
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
		PendulumPivot->SetRelativeRotation(FRotator::ZeroRotator);
		EnterLandedState();
	}
}

void APresent::EnterLandedState()
{
	// 서버가 확정한 위치로 스냅. 바닥에 붙지 않고 HoverHeight만큼 떠 있는 지점이다
	SetActorLocation(HoverLocation);

	// 낙하 스윕이 끝났으므로 메시 충돌은 더 이상 필요 없다. 획득 판정은 OverlapSphere가 담당한다.
	// 팝인 중 매 프레임 스케일이 바뀌는데, 충돌이 켜져 있으면 그때마다 콜리전 지오메트리를 다시 만든다.
	GiftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LandedElapsed = 0.f;

	// 스핀의 기준이 되는 기울기 자세. 이후 RotatingMovement가 여기에 월드 Yaw를 계속 얹는다
	SetActorRotation(FRotator(TiltPitchDeg, 0.f, TiltRollDeg));

	if (BalloonMesh)
	{
		BalloonMesh->SetVisibility(false, true);
	}

	if (GetNetMode() == NM_DedicatedServer)
	{
		// 데디케이티드 서버는 연출을 그릴 필요가 없다. 최종 상태로 확정하고 틱을 끈다.
		// OverlapSphere는 액터 원점에 있고 절대 스케일이므로 회전/스케일과 무관하게 획득 판정이 유지된다.
		SetActorScale3D(FVector::OneVector);
		SetActorTickEnabled(false);
		return;
	}

	RotatingMovement->Activate();

	SetActorScale3D(FVector::ZeroVector);
	PlayLandingVFX();
}

void APresent::TickLandedVisual(float DeltaTime)
{
	LandedElapsed += DeltaTime;

	// --- 팝인 스케일 (완료 후에는 1.0 고정) ---
	float Scale = 1.f;
	if (PopGrowDuration > 0.f && LandedElapsed < PopGrowDuration)
	{
		Scale = FMath::InterpEaseOut(0.f, PopOvershootScale, LandedElapsed / PopGrowDuration, 2.f);
	}
	else if (PopSettleDuration > 0.f && LandedElapsed < PopGrowDuration + PopSettleDuration)
	{
		const float SettleAlpha = (LandedElapsed - PopGrowDuration) / PopSettleDuration;
		Scale = FMath::InterpEaseInOut(PopOvershootScale, 1.f, SettleAlpha, 2.f);
	}
	SetActorScale3D(FVector(Scale));

	// --- 파도 출렁임 ---
	// 인스턴스마다 다른 위상을 주려고 복제된 ReplicatedSwayPhaseOffset을 재사용한다.
	// 선물 여러 개가 한 몸처럼 움직이는 것을 막는다.
	const float Phase = LandedElapsed * WaveSpeed + ReplicatedSwayPhaseOffset;

	// 착지 시 GiftMesh 충돌을 껐으므로 스윕 없이 이동해도 된다
	FVector WaveLocation = HoverLocation;
	WaveLocation.Z += SampleWaveCrest(Phase, WaveCrestSharpness) * WaveHeight;
	SetActorLocation(WaveLocation);

	// RotatingMovement는 월드 Z 요를 앞곱셈으로 누적한다:
	//   Qz(d) * Qz(Y)*Qy(P)*Qx(R) = Qz(Y+d)*Qy(P)*Qx(R)
	// 즉 Pitch/Roll은 보존되고 Yaw만 더해지므로, 누적된 Yaw를 읽어와 기울기만 덮어쓰면 된다.
	// 기울기 흔들림은 대칭이 자연스러우므로 마루를 뾰족하게 만들지 않은 원래 파형을 쓴다.
	const float AccumulatedYaw = GetActorRotation().Yaw;
	const float PitchWobble = WavePitchWobbleDeg * SampleWave(Phase * 0.83f + 2.1f);
	const float RollWobble = WaveRollWobbleDeg * SampleWave(Phase * 1.19f + 4.7f);
	SetActorRotation(FRotator(TiltPitchDeg + PitchWobble, AccumulatedYaw, TiltRollDeg + RollWobble));
}

void APresent::PlayLandingVFX()
{
	if (PopBurstVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, PopBurstVFX, HoverLocation, FRotator::ZeroRotator, FVector(1.f),
			true, true, ENCPoolMethod::None, true);
	}

	if (SparkleVFX && SparkleVFX->GetAsset())
	{
		SparkleVFX->Activate(true);

		// 지정된 시스템이 루프형이 아닐 때를 위한 재트리거. 루프 이미터라면 간격을 0으로 두면 된다
		if (SparkleRepeatInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(
				SparkleRepeatTimerHandle, this, &ThisClass::RestartSparkleVFX,
				SparkleRepeatInterval, true);
		}
	}
}

void APresent::RestartSparkleVFX()
{
	if (SparkleVFX && SparkleVFX->GetAsset())
	{
		SparkleVFX->Activate(true);
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
		if (PresentHitSoundEvent)
		{
			UAkGameplayStatics::PostEvent(PresentHitSoundEvent, nullptr, 0, FOnAkPostEventCallback());
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

void APresent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SparkleRepeatTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APresent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APresent, bHasLanded);
	DOREPLIFETIME(APresent, HoverLocation);
	DOREPLIFETIME(APresent, AnchorSpawnLocation);
	DOREPLIFETIME(APresent, ReplicatedSwayAxis);
	DOREPLIFETIME(APresent, ReplicatedSwayPhaseOffset);
}
