// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "DeveloperSettings/RagdollTestSettings.h"

URagdollTestSettings::URagdollTestSettings()
{
	// Current setup with and without latency, then the rotation snap off under latency where it used to hurt.
	// The baseline commit cannot be reproduced at runtime any more, its numbers are in Docs/RagdollNetworkSync.md
	struct FSetup { const TCHAR* Label; int32 Features; int32 Lag; };
	const FSetup Setups[] =
	{
		{ TEXT("current"), 57, 0 },
		{ TEXT("current_lag100"), 57, 100 },
		{ TEXT("nosnap_lag100"), 57 & ~1, 100 },
	};

	for (const FSetup& Setup : Setups)
	{
		FRagdollTestCase Case;
		Case.Label = Setup.Label;
		Case.Features = Setup.Features;
		Case.PktLag = Setup.Lag;
		Case.PktLagVariance = Setup.Lag > 0 ? 20 : 0;
		Case.PktLoss = Setup.Lag > 0 ? 2 : 0;
		Cases.Add(Case);
	}
}
