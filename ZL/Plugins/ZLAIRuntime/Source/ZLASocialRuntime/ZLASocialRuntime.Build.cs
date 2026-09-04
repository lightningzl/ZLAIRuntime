using UnrealBuildTool;

public class ZLASocialRuntime : ModuleRules
{
	public ZLASocialRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"DataRegistry",
			"DeveloperSettings"
		});
	}
}
