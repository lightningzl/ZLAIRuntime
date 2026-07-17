using UnrealBuildTool;

public class ZLAIRuntime : ModuleRules
{
	public ZLAIRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"HTTP",
			"Json"
		});
	}
}
