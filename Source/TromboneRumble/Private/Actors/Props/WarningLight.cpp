// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Props/WarningLight.h"
#include "Components/PointLightComponent.h"

AWarningLight::AWarningLight()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// It changes every frame during the warning, so nothing is baked
	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->SetMobility(EComponentMobility::Movable);
	SetRootComponent(PointLight);
}

void AWarningLight::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Every level actor gets here before any BeginPlay, so a gimmick that starts the warning from its BeginPlay finds the light ready
	BaseIntensity = PointLight->Intensity;

	// The light is placed on so it can be seen in the editor, and it stays dark in game until the first warning
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		TurnOff();
	}
}

void AWarningLight::StartWarning()
{
	if (GetNetMode() == NM_DedicatedServer) return;

	WarningStartTime = GetWorld()->GetTimeSeconds();
	FadeOutStartTime = -1.f;
	bFlickering = true;

	PointLight->SetVisibility(true);
	SetRatio(0.f);

	SetActorTickEnabled(true);
}

void AWarningLight::StopWarning()
{
	if (!bFlickering) return;

	bFlickering = false;
	FadeOutStartTime = GetWorld()->GetTimeSeconds();
	FadeOutStartRatio = CurrentRatio;
}

void AWarningLight::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFlickering)
	{
		UpdateFlicker();
	}
	else if (FadeOutStartTime >= 0.f)
	{
		UpdateFadeOut();
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

void AWarningLight::UpdateFlicker()
{
	const float Elapsed = GetWorld()->GetTimeSeconds() - WarningStartTime;
	const float Frequency = FMath::Max(FlickerFrequency, KINDA_SMALL_NUMBER);

	// The wave starts dark, so the light turns on smoothly
	const float Wave = 0.5f - 0.5f * FMath::Cos(2.f * PI * Frequency * Elapsed);

	// The first rise goes from 0 and every later swing from the minimum. Both meet at full brightness, so there is no jump
	const float HalfPeriod = 0.5f / Frequency;
	const float MinRatio = Elapsed < HalfPeriod ? 0.f : MinBrightnessRatio;
	SetRatio(FMath::Lerp(MinRatio, 1.f, Wave));
}

void AWarningLight::UpdateFadeOut()
{
	// The fade takes half a flicker, the time the flicker takes to go from bright to dark, so it moves at the same pace
	const float Duration = 0.5f / FMath::Max(FlickerFrequency, KINDA_SMALL_NUMBER);
	const float Alpha = (GetWorld()->GetTimeSeconds() - FadeOutStartTime) / Duration;

	if (Alpha >= 1.f)
	{
		TurnOff();
		SetActorTickEnabled(false);
		return;
	}

	// The same sine shape as the flicker, so the last dimming looks like one more swing of it
	SetRatio(FMath::InterpSinInOut(FadeOutStartRatio, 0.f, Alpha));
}

void AWarningLight::SetRatio(const float Ratio)
{
	CurrentRatio = Ratio;
	PointLight->SetIntensity(BaseIntensity * Ratio);
}

void AWarningLight::TurnOff()
{
	bFlickering = false;
	FadeOutStartTime = -1.f;
	CurrentRatio = 1.f;

	// The intensity goes back to the placed value, so the next warning starts from it
	PointLight->SetVisibility(false);
	PointLight->SetIntensity(BaseIntensity);
}
