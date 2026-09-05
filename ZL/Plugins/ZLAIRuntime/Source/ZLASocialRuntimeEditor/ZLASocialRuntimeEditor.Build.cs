using UnrealBuildTool;

public class ZLASocialRuntimeEditor : ModuleRules
{
	public ZLASocialRuntimeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UnrealEd", "ZLASocialRuntime", "ApplicationCore", "ContentBrowser", "DesktopPlatform", "PropertyEditor", "Slate", "SlateCore" });
	}
}
