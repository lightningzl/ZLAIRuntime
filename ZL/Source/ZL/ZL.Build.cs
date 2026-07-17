// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZL : ModuleRules
{
	public ZL(ReadOnlyTargetRules Target) : base(Target)
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

		PrivateDependencyModuleNames.AddRange(new string[] {
			"ZLAIRuntime"
		});

		PublicIncludePaths.AddRange(new string[] {
			"ZL",
			"ZL/Variant_Platforming",
			"ZL/Variant_Platforming/Animation",
			"ZL/Variant_Combat",
			"ZL/Variant_Combat/AI",
			"ZL/Variant_Combat/Animation",
			"ZL/Variant_Combat/Gameplay",
			"ZL/Variant_Combat/Interfaces",
			"ZL/Variant_Combat/UI",
			"ZL/Variant_SideScrolling",
			"ZL/Variant_SideScrolling/AI",
			"ZL/Variant_SideScrolling/Gameplay",
			"ZL/Variant_SideScrolling/Interfaces",
			"ZL/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
