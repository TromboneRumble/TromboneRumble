// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "DeveloperSettings/RagdollTestSettings.h"

URagdollTestSettings::URagdollTestSettings()
{
	// Current setup with and without latency. Add Overrides or a Sweep to tune, see Docs/RagdollNetworkSync.md 6-9
	struct FSetup { const TCHAR* Label; int32 Lag; };
	const FSetup Setups[] =
	{
		{ TEXT("current"), 0 },
		{ TEXT("current_lag100"), 100 },
	};

	for (const FSetup& Setup : Setups)
	{
		FRagdollTestCase Case;
		Case.Label = Setup.Label;
		Case.Features = 57;
		Case.PktLag = Setup.Lag;
		Case.PktLagVariance = Setup.Lag > 0 ? 20 : 0;
		Case.PktLoss = Setup.Lag > 0 ? 2 : 0;
		Cases.Add(Case);
	}
}

TArray<FRagdollTestCase> URagdollTestSettings::BuildCampaignCases() const
{
	TArray<FRagdollTestCase> Result = Cases;
	for (const FRagdollTestSweep& Sweep : Sweeps)
	{
		for (const float Value : Sweep.Values)
		{
			for (const int32 Lag : Sweep.Lags)
			{
				FRagdollTestCase Case;
				Case.Label = FString::Printf(TEXT("%s=%g%s"), *Sweep.Parameter.ToString(), Value, Lag > 0 ? *FString::Printf(TEXT("_lag%d"), Lag) : TEXT(""));
				Case.Features = Sweep.Features;
				Case.Drops = Sweep.Drops;
				Case.Launches = Sweep.Launches;
				Case.PktLag = Lag;
				Case.PktLagVariance = Lag > 0 ? 20 : 0;
				Case.PktLoss = Lag > 0 ? 2 : 0;
				Case.Overrides.Add(Sweep.Parameter, Value);
				Result.Add(Case);
			}
		}
	}
	return Result;
}
