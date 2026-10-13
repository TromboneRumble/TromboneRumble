// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WarningLight.generated.h"

class UPointLightComponent;

/**
 * WarningLight is an alarm light placed in the level. It stays off until a warning starts, then flickers on a sine wave.
 * When the warning ends it fades out from wherever the flicker is, at the same pace.
 * The intensity it is placed with is its brightest point.
 *
 * It does not know what sets it off. A gimmick finds the warning lights of the level and calls StartWarning and StopWarning.
 * Keep in mind that it draws nothing on a dedicated server.
 *
 * @see ABlackHoleGimmick
 */
UCLASS()
class TROMBONERUMBLE_API AWarningLight : public AActor
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	AWarningLight();

	/** Turn the light on and start the flicker from dark. Calling it again restarts the flicker. */
	void StartWarning();

	/** Fade the light out from wherever the flicker is, then turn it off. Does nothing while the warning is off. */
	void StopWarning();

protected:

	/** The light that flickers. Set its color, intensity and radius on the placed instance. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WarningLight")
	TObjectPtr<UPointLightComponent> PointLight;

	/** 경고등이 1초에 밝아졌다 어두워지는 횟수. 클수록 빠르게 깜빡인다 (Hz) */
	UPROPERTY(EditAnywhere, Category = "WarningLight", meta = (DisplayName = "깜빡임 속도", ClampMin = "0.1", Units = "Hz"))
	float FlickerFrequency = 1.f;

	/** 경고등이 가장 어두울 때의 밝기. 배치한 밝기를 1로 본 비율이며, 0이면 완전히 꺼졌다 켜진다 */
	UPROPERTY(EditAnywhere, Category = "WarningLight", meta = (DisplayName = "최소 밝기", ClampMin = "0.0", ClampMax = "1.0"))
	float MinBrightnessRatio = 0.2f;

private:

	/** Move the intensity along the sine wave. Runs every frame during the warning. */
	void UpdateFlicker();

	/** Dim toward 0, and turn the light off at the end. Runs every frame after the warning. */
	void UpdateFadeOut();

	/** Set the light to this part of the intensity it was placed with. */
	void SetRatio(float Ratio);

	/** Turn the light off at once and give it back the intensity it was placed with. */
	void TurnOff();

	/** Intensity the light was placed with. */
	float BaseIntensity = 0.f;

	/** Local world time the current warning started. The sine wave starts here. */
	float WarningStartTime = 0.f;

	/** Local world time the fade out started. Negative while the light does not fade. */
	float FadeOutStartTime = -1.f;

	/** Ratio the fade out starts from, so it continues from wherever the flicker was. */
	float FadeOutStartRatio = 1.f;

	/** Ratio last applied to the light. */
	float CurrentRatio = 1.f;

	/** Is the warning flickering, as opposed to fading out or off. */
	bool bFlickering = false;

public:

	//~ Begin AActor Interface
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface
};
