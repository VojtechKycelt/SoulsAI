// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SoulsAI : ModuleRules
{
	public SoulsAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"SoulsAI",
			"SoulsAI/Variant_Platforming",
			"SoulsAI/Variant_Platforming/Animation",
			"SoulsAI/Variant_Combat",
			"SoulsAI/Variant_Combat/AI",
			"SoulsAI/Variant_Combat/Animation",
			"SoulsAI/Variant_Combat/Gameplay",
			"SoulsAI/Variant_Combat/Interfaces",
			"SoulsAI/Variant_Combat/UI",
			"SoulsAI/Variant_SideScrolling",
			"SoulsAI/Variant_SideScrolling/AI",
			"SoulsAI/Variant_SideScrolling/Gameplay",
			"SoulsAI/Variant_SideScrolling/Interfaces",
			"SoulsAI/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
