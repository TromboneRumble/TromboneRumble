// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/Present/PresentSpawner.h"
#include "Actors/Gimmick/Present/Present.h"
#include "AkComponent.h"
#include "AkGameplayTypes.h"
#include "Engine/TargetPoint.h"

APresentSpawner::APresentSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SantaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SantaMesh"));
	SantaMesh->SetupAttachment(RootComponent);
	SantaMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	if (AkComponent)
	{
		AkComponent->OcclusionRefreshInterval = 0.f;
		AkComponent->SetupAttachment(RootComponent);
	}

	GimmickType = EGimmickType::Present;
	bReplicates = true;
	SetReplicateMovement(true);
}

void APresentSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void APresentSpawner::Activate()
{
	Super::Activate();
	StartPatrol();
}

void APresentSpawner::Deactivate()
{
	Super::Deactivate();

	bIsPatrolling = false;
	bMidPointReached = false;
	PatrolTargetLocation = FVector::ZeroVector;
	NextPatrolTargetLocation = FVector::ZeroVector;
	SetActorTickEnabled(false);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	SetActorHiddenInGame(false);

	Multicast_PlayJingleStop();
}

void APresentSpawner::StartPatrol()
{
	if (PatrolPointsA.IsEmpty() || MidPatrolPoints.IsEmpty() || PatrolPointsB.IsEmpty() || !PresentClass) return;

	// 시작 지점, 중간 경유지, 종착지점을 각각 랜덤 선택
	ATargetPoint* StartPoint = PatrolPointsA[FMath::RandRange(0, PatrolPointsA.Num() - 1)];
	ATargetPoint* MidPoint = MidPatrolPoints[FMath::RandRange(0, MidPatrolPoints.Num() - 1)];
	ATargetPoint* EndPoint = PatrolPointsB[FMath::RandRange(0, PatrolPointsB.Num() - 1)];

	if (!StartPoint || !MidPoint || !EndPoint) return;

	SetActorLocation(StartPoint->GetActorLocation());
	SetActorHiddenInGame(false);

	// 먼저 중간 경유지를 목표로 설정
	PatrolTargetLocation = MidPoint->GetActorLocation();
	NextPatrolTargetLocation = EndPoint->GetActorLocation();
	bMidPointReached = false;
	bIsPatrolling = true;
	SetActorTickEnabled(true);

	Multicast_PlayJingleStart();
}

void APresentSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || !bIsPatrolling) return;

	const FVector CurrentLocation = GetActorLocation();
	const FVector Direction = (PatrolTargetLocation - CurrentLocation).GetSafeNormal2D();

	// 산타 이동
	SetActorLocation(CurrentLocation + Direction * SantaSpeed * DeltaTime);

	// 이동 방향으로 산타 회전
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.ToOrientationRotator());
	}

	// 도달 거리 (100cm)
	const float ReachDistance = 100.f;

	if (!bMidPointReached)
	{
		// 중간 경유지 도달 감지
		if (FVector::Dist2D(CurrentLocation, PatrolTargetLocation) <= ReachDistance)
		{
			OnReachedMidPoint();
		}
	}
	else
	{
		// 최종 종착지점 도달 감지
		if (FVector::Dist2D(CurrentLocation, PatrolTargetLocation) <= ReachDistance)
		{
			OnReachedEndPoint();
		}
	}
}

void APresentSpawner::OnReachedMidPoint()
{
	// 중간 경유지에서 선물 드롭
	SpawnPresent(PatrolTargetLocation);

	// 이제 최종 종착지점으로 목표 변경
	PatrolTargetLocation = NextPatrolTargetLocation;
	bMidPointReached = true;
}

void APresentSpawner::OnReachedEndPoint()
{
	bIsPatrolling = false;
	bMidPointReached = false;
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);

	Multicast_PlayJingleStop();

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &APresentSpawner::RespawnSanta, RespawnDelay, false);
}

void APresentSpawner::RespawnSanta()
{
	if (!bIsActive) return;
	StartPatrol();
}

void APresentSpawner::SpawnPresent(const FVector& DropLocation)
{
	if (!HasAuthority() || !PresentClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform(FRotator::ZeroRotator, DropLocation);
	if (APresent* NewPresent = GetWorld()->SpawnActor<APresent>(PresentClass, SpawnTransform, SpawnParams))
	{
		NewPresent->BonusScore = PresentBonusScore;
	}

	Multicast_PlayHoHoHo();
}

void APresentSpawner::Multicast_PlayJingleStart_Implementation()
{
	if (AkComponent && JingleStartEvent)
	{
		AkComponent->PostAkEvent(JingleStartEvent, 0, FOnAkPostEventCallback());
	}
}

void APresentSpawner::Multicast_PlayJingleStop_Implementation()
{
	if (AkComponent && JingleStopEvent)
	{
		AkComponent->PostAkEvent(JingleStopEvent, 0, FOnAkPostEventCallback());
	}
}

void APresentSpawner::Multicast_PlayHoHoHo_Implementation()
{
	if (AkComponent && HoHoHoEvent)
	{
		AkComponent->PostAkEvent(HoHoHoEvent, 0, FOnAkPostEventCallback());
	}
}
