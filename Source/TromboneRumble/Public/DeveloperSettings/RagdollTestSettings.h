// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RagdollTestSettings.generated.h"

/** Editor mirror of ERagdollSyncFeature, so a case's Features shows as checkboxes. Values must match. */
UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ERagdollSyncFeatureFlags : uint8
{
	None = 0 UMETA(Hidden),
	RotationSnap = 1,
	Extrapolation = 8,
	RotationSync = 16,
	GravityTerm = 32,
};

/** One measurement setup: which sync features, how much fake latency, how many drops. */
USTRUCT()
struct FRagdollTestCase
{
	GENERATED_BODY()

	/** Written into every summary row of this case. */
	UPROPERTY(EditAnywhere)
	FString Label = TEXT("case");

	/** Sync parts on for this case. 57 = all. */
	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/TromboneRumble.ERagdollSyncFeatureFlags"))
	int32 Features = 57;

	/** Recorded lobby drops. Warm-up drops come on top. Five is enough for a stable median, each gives a self and an other row. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 Drops = 5;

	/** Recorded launches: ragdoll thrown up from standing, like a headbutt hit. Alternates with the drops. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 Launches = 5;

	/** Fake one-way latency (ms) on both the server and the client driver. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 PktLag = 0;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 PktLagVariance = 0;

	/** Packet loss (percent). */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "100"))
	int32 PktLoss = 0;

	/** Float properties of UTromboneRagdollComponent to set for this case, by name, e.g. MaxCorrectionSpeed 1200.
	 *  Applied to every ragdoll component in both PIE worlds, put back when the case ends. */
	UPROPERTY(EditAnywhere)
	TMap<FName, float> Overrides;
};

/** One parameter over several values, expanded into cases at campaign start. Each value gets one case per lag. */
USTRUCT()
struct FRagdollTestSweep
{
	GENERATED_BODY()

	/** Float property of UTromboneRagdollComponent, e.g. MaxCorrectionSpeed. */
	UPROPERTY(EditAnywhere)
	FName Parameter;

	UPROPERTY(EditAnywhere)
	TArray<float> Values;

	/** Fake one-way latencies to run every value under (ms). 0 = none. */
	UPROPERTY(EditAnywhere)
	TArray<int32> Lags = { 0, 100 };

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/TromboneRumble.ERagdollSyncFeatureFlags"))
	int32 Features = 57;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 Drops = 5;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 Launches = 5;
};

/** URagdollTestSettings
 *  Cases the ragdoll test campaign runs one after another. Project Settings > Game > Ragdoll Test.
 *  Start with Trombone_RagdollTest in any PIE window while the lobby waits in InstrumentScramble.
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Ragdoll Test"))
class TROMBONERUMBLE_API URagdollTestSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	URagdollTestSettings();

	static const URagdollTestSettings* Get() { return GetDefault<URagdollTestSettings>(); }

	UPROPERTY(Config, EditAnywhere, Category = "Campaign", meta = (TitleProperty = "Label"))
	TArray<FRagdollTestCase> Cases;

	/** Parameter sweeps, run after Cases. One case per value per lag, labeled Parameter=Value. */
	UPROPERTY(Config, EditAnywhere, Category = "Campaign", meta = (TitleProperty = "Parameter"))
	TArray<FRagdollTestSweep> Sweeps;

	/** @return Cases followed by every sweep expanded. What a campaign runs. */
	TArray<FRagdollTestCase> BuildCampaignCases() const;

	/** Drops at the start of each case that are measured but not recorded. Lets the new settings and the PIE warm-up settle. */
	UPROPERTY(Config, EditAnywhere, Category = "Campaign", meta = (ClampMin = "0"))
	int32 WarmupDrops = 1;

	/** Seconds from the last ragdoll ending to the next drop. Covers the get-up animation. */
	UPROPERTY(Config, EditAnywhere, Category = "Campaign", meta = (ClampMin = "0.0"))
	float DropInterval = 3.0f;

	/** Give up a drop when no ragdoll starts within this many seconds. Usually means the lobby is not in InstrumentScramble. */
	UPROPERTY(Config, EditAnywhere, Category = "Campaign", meta = (ClampMin = "1.0"))
	float DropTimeout = 8.0f;
};
