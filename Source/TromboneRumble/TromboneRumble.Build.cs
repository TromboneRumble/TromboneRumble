// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TromboneRumble : ModuleRules
{
	public TromboneRumble(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"OnlineSubsystem", 
			"OnlineSubsystemSteam", 
			"OnlineSubsystemUtils", 
			"GameplayTags", 
			"GameplayAbilities", 
			"GameplayTasks",
			"UMG", 
			"AkAudio", 
			"WwiseSoundEngine", 
			"Niagara", 
			"CommonUI",
			"CommonInput",
			"ApplicationCore",
			"EasySessions",
			"RHI",
			"MovieScene",
			"LevelSequence",
			"Lobby",
			"Projects",
			"Voice",
			"AudioCapture",
			"AudioCaptureCore",
			"AudioMixer"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayMessageRuntime", 
			"AsyncLoadingScreen", 
			"ProtoAnimatedText"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EngineSettings",
			"Slate",
			"SlateCore"
		});
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
    }

}
