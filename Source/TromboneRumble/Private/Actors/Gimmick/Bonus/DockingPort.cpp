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

ADockingPort::FFallProfile ADockingPort::GetFallProfile() const
{
	FFallProfile Fall;
	Fall.Distance = FMath::Max(FallStart.Z - FallEnd.Z, 0.f);
	Fall.Speed = FMath::Max(FallSpeed, 1.f);

	// The pod falls at a fixed speed, then slows down evenly over the brake height and stops on the floor.
	// Slowing from Speed to 0 over BrakeDistance takes twice as long as crossing it at full speed
	const float BrakeDistance = FMath::Min(BrakeHeight, Fall.Distance);
	Fall.CruiseTime = (Fall.Distance - BrakeDistance) / Fall.Speed;
	Fall.BrakeTime = 2.f * BrakeDistance / Fall.Speed;
	return Fall;
}

bool ADockingPort::EvaluateFall(const float Elapsed, FVector& OutLocation) const
{
	const FFallProfile Fall = GetFallProfile();

	float Travelled;
	if (Elapsed < Fall.CruiseTime)
	{
		Travelled = Fall.Speed * Elapsed;
	}
	else if (Elapsed < Fall.TotalTime())
	{
		const float BrakeElapsed = Elapsed - Fall.CruiseTime;
		const float Deceleration = Fall.Speed / Fall.BrakeTime;
		Travelled = Fall.Speed * Fall.CruiseTime + Fall.Speed * BrakeElapsed - 0.5f * Deceleration * BrakeElapsed * BrakeElapsed;
	}
	else
	{
		OutLocation = FallEnd;
		return true;
	}

	OutLocation = Fall.Distance > KINDA_SMALL_NUMBER ? FMath::Lerp(FallStart, FallEnd, Travelled / Fall.Distance) : FallEnd;
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
	if (!bLandingVFXPlayed && Elapsed >= GetFallProfile().TotalTime() - LandingVFXOffset)
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

void ADockingPort::OnExitStarted()
{
	ExitFallTime = GetFallTimeToReverse();
	ExitStartTime = GetWorld()->GetTimeSeconds();

	// The thrusters fire again for the launch, and the glow is cut at once so no particle stays behind on the way up
	GetWorldTimerManager().ClearTimer(GlowTimerHandle);
	GlowVFXComponent->DeactivateImmediate();
	if (LandingVFXComponent->GetAsset())
	{
		LandingVFXComponent->Activate(true);
	}
}

void ADockingPort::TickExit(const float DeltaTime)
{
	// The launch is the fall played backwards, so the fall time runs down from where the pod was and stops at the start
	const float Elapsed = GetWorld()->GetTimeSeconds() - ExitStartTime;

	FVector Location;
	EvaluateFall(FMath::Max(ExitFallTime - Elapsed, 0.f), Location);
	SetActorLocation(Location);
}

float ADockingPort::GetExitDuration() const
{
	return GetFallTimeToReverse();
}

float ADockingPort::GetFallTimeToReverse() const
{
	const float FallTime = GetFallProfile().TotalTime();

	if (bHasLanded) return FallTime;

	// Picked up on the way down, so the pod goes back up only as far as it has come
	const float Elapsed = UTromboneStatics::GetServerWorldTime(this) - FallStartServerTime;
	return FMath::Clamp(Elapsed, 0.f, FallTime);
}

void ADockingPort::PlayLandingVFX()
{
	if (bLandingVFXPlayed) return;
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
