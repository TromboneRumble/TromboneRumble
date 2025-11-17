// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/SpotlightZone.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/DebugHelper.h"

ASpotlightZone::ASpotlightZone()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComponent"));
	SpotLightComponent->SetupAttachment(RootComponent);
	SpotLightComponent->SetIntensity(50000.0f);
	SpotLightComponent->SetAttenuationRadius(600.0f);
	SpotLightComponent->SetInnerConeAngle(5.0f);
	SpotLightComponent->SetOuterConeAngle(20.0f);
	SpotLightComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); 
	SpotLightComponent->SetVisibility(false);

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(RootComponent);
	DecalComponent->DecalSize = FVector(700.0f, 200.0f, 200.0f);
	DecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); 
	DecalComponent->SetVisibility(false);

	LightBeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightBeamMesh"));
	LightBeamMesh->SetupAttachment(RootComponent);
	LightBeamMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	LightBeamMesh->SetCastShadow(false);
	LightBeamMesh->SetVisibility(false);

	bReplicates = true;
	AActor::SetReplicateMovement(false);
}

void ASpotlightZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentState);
	DOREPLIFETIME(ThisClass, bIsBonusAwarded);
}

void ASpotlightZone::InitializeZone(const bool bIsFeverTime)
{
	if (!HasAuthority()) return;

	if (bIsFeverTime)
	{
		SetState(ESpotlightState::Active);
	}
	else
	{
		SetState(ESpotlightState::Warning);
	}
}

bool ASpotlightZone::AttemptToAwardBonus(ADefaultTromboneCharacter* Player)
{
	if (!HasAuthority()) return false;

	if (CurrentState == ESpotlightState::Active && !bIsBonusAwarded)
	{
		bIsBonusAwarded = true;
		SetState(ESpotlightState::Awarded);
		const FString DebugMsg = FString::Printf(TEXT("SpotlightZone '%s': Bonus awarded to player '%s'"), *GetName(), *Player->GetName());
		PRINT_WITH_CURRENT_CONTEXT(DebugMsg);
		return true;
	}

	return false;
}

void ASpotlightZone::SetState(ESpotlightState NewState)
{
	if (!HasAuthority()) return;
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	OnRep_CurrentState();

	GetWorldTimerManager().ClearTimer(LifecycleTimerHandle);

	switch (CurrentState)
	{
		case ESpotlightState::Warning:
			StartLifecycleTimer(WarningDuration, &ASpotlightZone::OnWarningFinished);
			break;

		case ESpotlightState::Active:
			StartLifecycleTimer(ActiveDuration, &ASpotlightZone::OnActiveFinished);
			break;

		case ESpotlightState::Awarded:
			StartLifecycleTimer(AwardedDuration, &ASpotlightZone::OnAwardedFinished);
			break;

		case ESpotlightState::Fading:
			StartLifecycleTimer(AwardedDuration, &ASpotlightZone::OnFadingFinished);
			break;

		case ESpotlightState::None:
		default:
			break;
	}
}

void ASpotlightZone::StartLifecycleTimer(const float InDuration, void(ASpotlightZone::* InTimerMethod)())
{
	GetWorldTimerManager().SetTimer(LifecycleTimerHandle, this, InTimerMethod, InDuration, false);
}

void ASpotlightZone::OnWarningFinished()
{
	SetState(ESpotlightState::Active);
}

void ASpotlightZone::OnActiveFinished()
{
	SetState(ESpotlightState::Fading);
}

void ASpotlightZone::OnAwardedFinished()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

void ASpotlightZone::OnFadingFinished()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

void ASpotlightZone::OnRep_CurrentState()
{
	switch (CurrentState)
	{
		case ESpotlightState::Warning:
			SpotLightComponent->SetVisibility(false);
			DecalComponent->SetVisibility(true);
			DecalComponent->SetMaterial(0, WarningMaterial);
			LightBeamMesh->SetVisibility(false);
			break;
		
		case ESpotlightState::Active:
			SpotLightComponent->SetVisibility(true);
			DecalComponent->SetVisibility(true);
			DecalComponent->SetMaterial(0, ActiveMaterial);
			LightBeamMesh->SetVisibility(true);
			break;
		
		case ESpotlightState::Awarded:
			SpotLightComponent->SetVisibility(false);
			DecalComponent->SetVisibility(false);
			LightBeamMesh->SetVisibility(false);
			break;
		
		case ESpotlightState::Fading:
			SpotLightComponent->SetVisibility(false);
			DecalComponent->SetVisibility(false);
			LightBeamMesh->SetVisibility(false);
			break;
		
		case ESpotlightState::None:
		default:
			SpotLightComponent->SetVisibility(false);
			DecalComponent->SetVisibility(false);
			LightBeamMesh->SetVisibility(false);
			break;
	}
}