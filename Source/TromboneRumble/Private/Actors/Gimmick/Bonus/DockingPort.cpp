// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Bonus/DockingPort.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Gimmick/DockingPortGimmickConfig.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Utilities/TromboneStatics.h"

ADockingPort::ADockingPort()
{
	PortMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortMesh"));
	SetRootComponent(PortMesh);
	// The fall is computed, not swept, so the mesh needs no collision
	PortMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	OverlapSphere->SetupAttachment(PortMesh);

	LandingVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LandingVFXComponent"));
	LandingVFXComponent->SetupAttachment(PortMesh);
	LandingVFXComponent->SetAutoActivate(false);

	GlowVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GlowVFXComponent"));
	GlowVFXComponent->SetupAttachment(PortMesh);
	GlowVFXComponent->SetAutoActivate(false);

	ScoreType = EScoreType::DockingPort;
}

void ADockingPort::ApplyConfig(const UBonusGimmickConfig& Config)
{
	Super::ApplyConfig(Config);

	// The spawner of another gimmick type can drop a docking port too, and then it keeps the defaults
	if (const UDockingPortGimmickConfig* DockingConfig = Cast<UDockingPortGimmickConfig>(&Config))
	{
		FallSpeed = DockingConfig->FallSpeed;
		BrakeHeight = DockingConfig->BrakeHeight;
	}
}

void ADockingPort::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	FallStart = GetActorLocation();

	// With no floor below, the pod lands where it spawned instead of falling forever
	FVector Ground;
	FallEnd = FindGroundBelow(GetWorld(), FallStart, Ground) ? Ground + FVector(0.f, 0.f, LandOffsetZ) : FallStart;

	FallStartServerTime = UTromboneStatics::GetServerWorldTime(this);
}

void ADockingPort::GetFallTimes(float& OutCruiseTime, float& OutBrakeTime) const
{
	const float Distance = FMath::Max(FallStart.Z - FallEnd.Z, 0.f);
	const float Speed = FMath::Max(FallSpeed, 1.f);

	// The pod falls at a fixed speed, then slows down evenly over the brake height and stops on the floor.
	// Slowing from Speed to 0 over BrakeDistance takes twice as long as crossing it at full speed
	const float BrakeDistance = FMath::Min(BrakeHeight, Distance);
	OutCruiseTime = (Distance - BrakeDistance) / Speed;
	OutBrakeTime = 2.f * BrakeDistance / Speed;
}

bool ADockingPort::EvaluateFall(const float Elapsed, FVector& OutLocation) const
{
	const float Distance = FMath::Max(FallStart.Z - FallEnd.Z, 0.f);
	const float Speed = FMath::Max(FallSpeed, 1.f);

	float CruiseTime, BrakeTime;
	GetFallTimes(CruiseTime, BrakeTime);
	const float CruiseDistance = CruiseTime * Speed;

	float Travelled;
	if (Elapsed < CruiseTime)
	{
		Travelled = Speed * Elapsed;
	}
	else if (Elapsed < CruiseTime + BrakeTime)
	{
		const float BrakeElapsed = Elapsed - CruiseTime;
		const float Deceleration = Speed / BrakeTime;
		Travelled = CruiseDistance + Speed * BrakeElapsed - 0.5f * Deceleration * BrakeElapsed * BrakeElapsed;
	}
	else
	{
		OutLocation = FallEnd;
		return true;
	}

	OutLocation = Distance > KINDA_SMALL_NUMBER ? FMath::Lerp(FallStart, FallEnd, Travelled / Distance) : FallEnd;
	return false;
}

void ADockingPort::TickFalling(const float DeltaTime)
{
	// The fall values have not arrived yet
	if (FallStartServerTime < 0.f) return;

	// The server time of a client can run slightly behind the server, and a negative time would draw the pod above its start
	const float Elapsed = FMath::Max(UTromboneStatics::GetServerWorldTime(this) - FallStartServerTime, 0.f);

	FVector Location;
	const bool bReachedFloor = EvaluateFall(Elapsed, Location);
	SetActorLocation(Location);

	// The landing effect starts a little before the floor, so the dust is already there when the pod touches it
	float CruiseTime, BrakeTime;
	GetFallTimes(CruiseTime, BrakeTime);
	if (!bLandingVFXPlayed && Elapsed >= CruiseTime + BrakeTime - LandingVFXOffset)
	{
		PlayLandingVFX();
	}

	// Clients wait at the floor until the server says the pod has landed
	if (bReachedFloor && HasAuthority())
	{
		Land(FallEnd);
	}
}

void ADockingPort::OnLanded()
{
	SetActorLocation(LandedLocation);

	if (GetNetMode() == NM_DedicatedServer) return;

	PlayLandingVFX();

	if (GlowVFXOffset <= 0.f)
	{
		StartGlowVFX();
	}
	else
	{
		GetWorldTimerManager().SetTimer(GlowTimerHandle, this, &ThisClass::StartGlowVFX, GlowVFXOffset, false);
	}
}

void ADockingPort::PlayLandingVFX()
{
	if (bLandingVFXPlayed || GetNetMode() == NM_DedicatedServer) return;
	bLandingVFXPlayed = true;

	if (LandingVFXComponent->GetAsset())
	{
		LandingVFXComponent->Activate();
	}
}

void ADockingPort::StartGlowVFX()
{
	if (GlowVFXComponent->GetAsset())
	{
		GlowVFXComponent->Activate();
	}
}

void ADockingPort::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, FallStart);
	DOREPLIFETIME(ThisClass, FallEnd);
	DOREPLIFETIME(ThisClass, FallStartServerTime);
	DOREPLIFETIME(ThisClass, FallSpeed);
	DOREPLIFETIME(ThisClass, BrakeHeight);
}
