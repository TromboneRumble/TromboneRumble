// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TromboneRumble : ModuleRules
{
	public TromboneRumble(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "OnlineSubsystem", "OnlineSubsystemSteam", "OnlineSubsystemUtils", "GameplayTags", "GameplayAbilities", "GameplayTasks",
			"UMG", "AkAudio", "WwiseSoundEngine", "Niagara", "CommonUI", "CommonInput", "ApplicationCore", "EasySessions", "RHI", "MovieScene", "LevelSequence",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {"GameplayMessageRuntime", "AsyncLoadingScreen", "ProtoAnimatedText"});

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }

}
