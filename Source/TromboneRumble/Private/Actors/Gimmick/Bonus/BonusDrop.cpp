// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Bonus/BonusDrop.h"
#include "AkGameplayStatics.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/ActorComponents/ClientToServerRelayComponent.h"
#include "Components/SphereComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "Data/Gimmick/BonusGimmickConfig.h"
#include "Net/UnrealNetwork.h"

namespace
{
	/** Seconds the drop stays after its exit is over. */
	constexpr float ExitRemoveDelay = 0.2f;
}

ABonusDrop::ABonusDrop()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	AActor::SetReplicateMovement(false);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetSphereRadius(80.f);
	OverlapSphere->SetCollisionProfileName(TEXT("Trigger"));
	// The subclass pops the root in by scale, and a relative scale would shrink the pickup area with it
	OverlapSphere->SetUsingAbsoluteScale(true);
}

bool ABonusDrop::FindGroundBelow(const UWorld* World, const FVector& From, FVector& OutGround)
{
	if (!World) return false;

	FHitResult Hit;
	const FVector To = From - FVector(0.f, 0.f, 100000.f);
	if (!World->LineTraceSingleByObjectType(Hit, From, To, FCollisionObjectQueryParams(ECC_WorldStatic)))
	{
		return false;
	}

	OutGround = Hit.ImpactPoint;
	return true;
}

void ABonusDrop::ApplyConfig(const UBonusGimmickConfig& Config)
{
	BonusScore = Config.BonusScore;
	Lifetime = Config.Lifetime;
}

void ABonusDrop::BeginPlay()
{
	Super::BeginPlay();

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleOverlapBegin);
}

void ABonusDrop::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bExiting)
	{
		TickExit(DeltaTime);
	}
	else if (bHasLanded)
	{
		TickLanded(DeltaTime);
	}
	else
	{
		TickFalling(DeltaTime);
	}
}

void ABonusDrop::Land(const FVector& InLandedLocation)
{
	if (!HasAuthority() || bHasLanded) return;

	bHasLanded = true;
	LandedLocation = InLandedLocation;

	// The lifetime counts from the landing, so a long fall does not eat into it
	if (Lifetime > 0.f)
	{
		GetWorldTimerManager().SetTimer(LifetimeTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::StartExit, static_cast<ACharacter*>(nullptr)), Lifetime, false);
	}

	OnRep_HasLanded();
}

void ABonusDrop::OnRep_HasLanded()
{
	if (!bHasLanded) return;

	OnLanded();
	K2_OnLanded();
}

void ABonusDrop::HandleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCollectedLocally) return;

	// Each player picks up on its own machine, the same way every other score is added
	ADefaultTromboneCharacter* Character = Cast<ADefaultTromboneCharacter>(OtherActor);
	if (!Character || !Character->IsLocallyControlled() || !Character->IsPlayerControlled()) return;

	ADefaultPlayerState* PlayerState = Character->GetPlayerState<ADefaultPlayerState>();
	if (!PlayerState) return;

	bCollectedLocally = true;
	PlayerState->AddScore(BonusScore, ScoreType);

	// Played here and not in the multicast, so the collector hears it without waiting for the server
	if (CollectSound)
	{
		UAkGameplayStatics::PostEvent(CollectSound, nullptr, 0, FOnAkPostEventCallback());
	}

	// A client does not own this actor, so it asks the server through the relay component of its own character
	if (HasAuthority())
	{
		StartExit(Character);
	}
	else if (UClientToServerRelayComponent* Relay = Character->GetClientToServerRelayComponent())
	{
		Relay->Server_SendRPCRequest(this);
	}
}

void ABonusDrop::HandleServerRPC(ACharacter* InstigatorCharacter)
{
	if (!HasAuthority() || !InstigatorCharacter) return;

	StartExit(InstigatorCharacter);
}

void ABonusDrop::StartExit(ACharacter* Collector)
{
	// The multicast runs here at once, so a second pickup or the lifetime sees the exit already started
	if (bExiting) return;

	GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
	Multicast_ExitStarted(Collector);

	// Removing the drop closes its actor channel.
	// So it waits for the multicast to leave, and for a client that got the multicast late to finish the exit
	SetLifeSpan(GetExitDuration() + ExitRemoveDelay);
}

void ABonusDrop::Multicast_ExitStarted_Implementation(ACharacter* Collector)
{
	// Nobody else picks it up while it leaves
	bExiting = true;
	bCollectedLocally = true;
	SetActorEnableCollision(false);

	// The lifetime ends without a collector
	if (Collector)
	{
		OnCollected(Collector);
	}

	OnExitStarted();
}

void ABonusDrop::OnExitStarted()
{
	SetActorHiddenInGame(true);
}

void ABonusDrop::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BonusScore);
	DOREPLIFETIME(ThisClass, bHasLanded);
	DOREPLIFETIME(ThisClass, LandedLocation);
}
