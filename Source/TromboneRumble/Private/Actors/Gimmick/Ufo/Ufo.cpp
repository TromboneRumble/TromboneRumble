// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Ufo/Ufo.h"
#include "Actors/Gimmick/Ufo/UfoGimmick.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Data/Gimmick/UfoGimmickConfig.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Utilities/TromboneLogs.h"
#include "Utilities/TromboneStatics.h"

namespace
{
	/** Flight height the Blueprint tunes BeamVFXZScale at. */
	constexpr float BeamVFXReferenceHeight = 600.f;
}

namespace UfoWarp
{
	/** Seconds the body bounces after the UFO stops. */
	constexpr float SettleDuration = 0.3f;

	/** Seconds the body squashes before the UFO dashes away. */
	constexpr float WindupDuration = 0.2f;

	/** Extra length along the flight at full warp speed. */
	constexpr float StretchLength = 0.6f;

	/** Width lost at full warp speed. */
	constexpr float StretchThin = 0.3f;

	/** Height lost at the deepest squash. */
	constexpr float SquashAmount = 0.25f;

	/** Part of the warp the body takes to grow from nothing, or to shrink to nothing. */
	constexpr float GrowPart = 0.3f;
}

AUfo::AUfo()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	BodyPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BodyPivot"));
	BodyPivot->SetupAttachment(GetRootComponent());

	// ApplyBeamState turns the overlap on, on the server only. The effect draws the beam, so the mesh only shows in the editor
	BeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamMesh"));
	BeamMesh->SetupAttachment(GetRootComponent());
	BeamMesh->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamMesh->SetGenerateOverlapEvents(true);
	BeamMesh->SetCastShadow(false);
	BeamMesh->SetHiddenInGame(true);

	// Off until the beam spreads. UpdateMotion turns it on and off
	BeamVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BeamVFXComponent"));
	BeamVFXComponent->SetupAttachment(GetRootComponent());
	BeamVFXComponent->SetAutoActivate(false);
}

const UUfoGimmickConfig& AUfo::GetConfig() const
{
	// The owner replicates, so clients reach the same config. The Blueprint editor has no owner and shows the defaults
	const AUfoGimmick* Gimmick = Cast<AUfoGimmick>(GetOwner());
	return Gimmick ? Gimmick->GetUfoConfig() : *GetDefault<UUfoGimmickConfig>();
}

void AUfo::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Also runs in the Blueprint editor, so the beam shows at its real size there
	FitBeamToLength();
}

void AUfo::BeginPlay()
{
	Super::BeginPlay();

	// The UFO faces along its line, so the warp can stretch the body along the local X axis
	SetActorRotation(GetTravelDirection().Rotation());

	// The beam spreads only after the warp in. A client may skip the construction of a replicated actor, so this runs here too
	FitBeamToLength(0.f);
	UpdateMotion(UTromboneStatics::GetServerWorldTime(this));

	// The Blueprint is ready only now, so the first beam event waits until here
	ApplyBeamState();
}

void AUfo::FitBeamToLength(const float Fraction)
{
	const float Length = GetConfig().BeamLength * FMath::Clamp(Fraction, 0.f, 1.f);
	const UStaticMesh* Mesh = BeamMesh->GetStaticMesh();
	const float MeshHeight = Mesh ? Mesh->GetBoundingBox().Max.Z : 0.f;

	// A zero scale breaks the collision, so a beam with no length is hidden instead
	const bool bHasLength = MeshHeight > UE_KINDA_SMALL_NUMBER && Length > 1.f;
	BeamMesh->SetVisibility(bHasLength);
	if (!bHasLength) return;

	// The pivot sits at the bottom of the mesh, so the mesh goes to the end of the beam and grows up to the root
	// Only the height follows the length. The width stays as the Blueprint set it
	FVector Scale = BeamMesh->GetRelativeScale3D();
	Scale.Z = Length / MeshHeight;
	BeamMesh->SetRelativeScale3D(Scale);
	BeamMesh->SetRelativeLocation(FVector(0.f, 0.f, -Length));
}

void AUfo::ApplyBeamState()
{
	// UpdateMotion shows and hides the mesh as the beam spreads and folds. The catch follows only the full beam
	BeamMesh->SetCollisionEnabled(HasAuthority() && bBeamOn ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	OnBeamChanged(bBeamOn);
}

FVector AUfo::GetTravelDirection() const
{
	const FVector Direction = (Path.End - Path.Start).GetSafeNormal2D();
	return Direction.IsNearlyZero() ? GetActorForwardVector() : Direction;
}

void AUfo::UpdateMotion(const float Now)
{
	const UUfoGimmickConfig& Config = GetConfig();
	const FVector Direction = GetTravelDirection();
	const float Elapsed = Now - Path.StartServerTime;
	const float LineStart = Config.GetIntroDuration();
	const float LineEnd = LineStart + Path.Duration;

	FVector Location = Path.Start;
	float Stretch = 0.f;  // 1 at full warp speed
	float Squash = 0.f;   // Positive flattens the body
	float Size = 1.f;     // Grows in and shrinks out at the far end of the warp
	float BeamFraction = 0.f;

	if (LeaveStartServerTime >= 0.f && Now >= LeaveStartServerTime)
	{
		const float Since = Now - LeaveStartServerTime;
		Location = Path.End;

		if (Since < UfoWarp::WindupDuration)
		{
			// Crouch before the dash
			Squash = FMath::Sin(PI * Since / UfoWarp::WindupDuration);
		}
		else
		{
			if (!bWarpOutFired)
			{
				bWarpOutFired = true;
				OnWarpOut();
			}

			// Speeds up away from the line, the reverse of the arrival
			const float Alpha = FMath::Clamp((Since - UfoWarp::WindupDuration) / Config.WarpDuration, 0.f, 1.f);
			Location += Direction * Config.WarpDistance * Alpha * Alpha * Alpha;
			Stretch = Alpha * Alpha;
			Size = FMath::Clamp((1.f - Alpha) / UfoWarp::GrowPart, 0.f, 1.f);
		}
	}
	else if (Elapsed < Config.WarpDuration)
	{
		// Comes in fast from behind the line and brakes hard at its start
		const float Alpha = FMath::Clamp(Elapsed / Config.WarpDuration, 0.f, 1.f);
		const float Remaining = 1.f - Alpha;
		Location -= Direction * Config.WarpDistance * Remaining * Remaining * Remaining;
		Stretch = Remaining * Remaining;
		Size = FMath::Clamp(Alpha / UfoWarp::GrowPart, 0.f, 1.f);
	}
	else
	{
		if (!bWarpInFired)
		{
			bWarpInFired = true;
			OnWarpIn();
		}

		// The line starts once the beam is down. Before that the path holds the UFO at its start
		Location = Path.Evaluate(Now - LineStart);

		// A fading wobble after the hard stop
		const float Settle = (Elapsed - Config.WarpDuration) / UfoWarp::SettleDuration;
		if (Settle < 1.f)
		{
			Squash = FMath::Sin(Settle * 2.5f * PI) * (1.f - Settle);
		}

		const float Deploy = FMath::Max(Config.BeamDeployDuration, UE_KINDA_SMALL_NUMBER);
		BeamFraction = Elapsed < LineEnd
			? (Elapsed - Config.WarpDuration) / Deploy
			: 1.f - (Elapsed - LineEnd) / Deploy;
	}

	SetActorLocation(Location);
	FitBeamToLength(BeamFraction);

	// The effect plays from the start of the spread to the start of the fold, and animates its own in and out
	const bool bLeaving = LeaveStartServerTime >= 0.f && Now >= LeaveStartServerTime;
	SetBeamVFXOn(!bLeaving && Elapsed >= Config.WarpDuration && Elapsed < LineEnd);

	// Deactivate only stops the spawning, and a long lived particle would hang there until the UFO is gone.
	// So once the fold is over, whatever is left of the beam is cut
	if (!bBeamVFXOn && !bBeamVFXCut && Elapsed >= LineEnd + Config.BeamDeployDuration)
	{
		bBeamVFXCut = true;
		BeamVFXComponent->DeactivateImmediate();
	}

	const FVector BodyScale(
		1.f + UfoWarp::StretchLength * Stretch + UfoWarp::SquashAmount * Squash,
		1.f - UfoWarp::StretchThin * Stretch + UfoWarp::SquashAmount * Squash,
		1.f - UfoWarp::StretchThin * Stretch - UfoWarp::SquashAmount * Squash);
	BodyPivot->SetRelativeScale3D(BodyScale * FMath::Max(Size, KINDA_SMALL_NUMBER));
}

void AUfo::SetBeamVFXOn(const bool bOn)
{
	if (bBeamVFXOn == bOn || GetNetMode() == NM_DedicatedServer) return;
	bBeamVFXOn = bOn;

	if (!bOn)
	{
		BeamVFXComponent->Deactivate();
		return;
	}

	// The origin of the effect is the bottom of its beam, like the pivot of the mesh, so it stands on the floor and reaches up to the body.
	// Only Z is scaled, so the beam keeps its width. The emitters simulate in local space, so the scale reaches the particles.
	// The Blueprint tunes the scale at the reference height, and a taller flight stretches the beam by the same ratio
	const float BeamLength = GetConfig().BeamLength;
	const float ZScale = BeamVFXZScale * BeamLength / BeamVFXReferenceHeight;
	BeamVFXComponent->SetRelativeLocation(FVector(0.f, 0.f, -BeamLength));
	BeamVFXComponent->SetRelativeScale3D(FVector(1.f, 1.f, ZScale));
	BeamVFXComponent->Activate();
}

void AUfo::UpdateServer(const float Now)
{
	const UUfoGimmickConfig& Config = GetConfig();
	const float Elapsed = Now - Path.StartServerTime;
	const float LineStart = Config.GetIntroDuration();
	const float LineEnd = LineStart + Path.Duration;

	if (!bBeamFinished)
	{
		// Catches only once the beam has spread all the way down
		if (!bBeamOn && Elapsed >= LineStart)
		{
			SetBeamOn(true);
		}
		if (!bBeamOn) return;

		// Every tick and not only on begin overlap, so a player who was stunned when the beam reached them is caught once the stun ends
		CatchCharactersInBeam();

		// The beam starts to fold here, and nobody hangs from a beam that is gone
		if (Elapsed >= LineEnd)
		{
			SetBeamOn(false);
			bBeamFinished = true;
			ReleaseAllLifted();
		}
		return;
	}

	// Leaves once the beam has folded up
	if (LeaveStartServerTime >= 0.f || Elapsed < LineEnd + Config.BeamDeployDuration) return;

	LeaveStartServerTime = Now;

	// Every client finishes the dash on its own copy and removes it, so a slow connection never sees the UFO vanish mid dash
	TearOff();
	SetLifeSpan(UfoWarp::WindupDuration + Config.WarpDuration);
}

void AUfo::TornOff()
{
	Super::TornOff();

	// The start of the leave comes in the same update as the tear off. Starting now is the fallback when it did not
	const float Now = UTromboneStatics::GetServerWorldTime(this);
	if (LeaveStartServerTime < 0.f)
	{
		LeaveStartServerTime = Now;
	}

	const float LeaveEnd = LeaveStartServerTime + UfoWarp::WindupDuration + GetConfig().WarpDuration;
	SetLifeSpan(FMath::Max(LeaveEnd - Now, 0.1f));
}

void AUfo::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Now = UTromboneStatics::GetServerWorldTime(this);
	UpdateMotion(Now);

	// A torn off copy on a client counts as authority too, and it must not run the server rules
	if (HasAuthority() && !GetTearOff())
	{
		UpdateServer(Now);
	}
}

void AUfo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Destroyed early, for example when the round ends. Let go of everyone
	if (HasAuthority())
	{
		ReleaseAllLifted();
	}

	Super::EndPlay(EndPlayReason);
}

void AUfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Path);
	DOREPLIFETIME(ThisClass, bBeamOn);
	DOREPLIFETIME(ThisClass, LeaveStartServerTime);
}

void AUfo::CatchCharactersInBeam()
{
	TArray<AActor*> Overlapping;
	BeamMesh->GetOverlappingActors(Overlapping, ATromboneCharacterBase::StaticClass());

	for (AActor* Actor : Overlapping)
	{
		ATromboneCharacterBase* Character = Cast<ATromboneCharacterBase>(Actor);
		if (!Character || !Character->CanBeBeamLifted()) continue;

		LiftedCharacters.AddUnique(Character);
		Character->StartBeamLift(this);
		Multicast_OnCharacterLifted(Character);

		UE_LOG(LogGimmick, Log, TEXT("[UFO] Caught %s"), *Character->GetName());
	}
}

void AUfo::ReleaseAllLifted()
{
	// EndBeamLift calls NotifyBeamLiftEnded, which removes from the list, so we iterate a copy
	const TArray<TWeakObjectPtr<ATromboneCharacterBase>> Lifted = MoveTemp(LiftedCharacters);
	for (const TWeakObjectPtr<ATromboneCharacterBase>& WeakCharacter : Lifted)
	{
		if (ATromboneCharacterBase* Character = WeakCharacter.Get())
		{
			Character->EndBeamLift();
		}
	}
}

void AUfo::NotifyBeamLiftEnded(ATromboneCharacterBase* Character)
{
	LiftedCharacters.Remove(Character);
}

void AUfo::SetBeamOn(const bool bOn)
{
	if (bBeamOn == bOn) return;

	bBeamOn = bOn;
	ApplyBeamState();
}

void AUfo::OnRep_BeamOn()
{
	// The first value arrives with the actor and BeginPlay reports it
	if (HasActorBegunPlay())
	{
		ApplyBeamState();
	}
}

void AUfo::Multicast_OnCharacterLifted_Implementation(ATromboneCharacterBase* Character)
{
	OnCharacterLifted(Character);
}
