// Copyright (C) 2026 biksari studio. All Rights Reserved.

using UnrealBuildTool;

public class TromboneRumbleEditor : ModuleRules
{
	public TromboneRumbleEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"PropertyEditor",
			"WorkspaceMenuStructure",
			"TromboneRumble"
		});
	}
}
