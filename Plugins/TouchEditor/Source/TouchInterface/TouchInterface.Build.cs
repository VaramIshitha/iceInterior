// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TouchInterface : ModuleRules
{
	public TouchInterface(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"TouchInterface/Public/Classes",
				"TouchInterface/Public/Components",
				"TouchInterface/Public/Helpers",
				"TouchInterface/Public/SaveSystem",
				"TouchInterface/Public/Settings",
				"TouchInterface/Public/TouchInterface"
				// ... add other private include paths required here ...
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"AssetRegistry",
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"ApplicationCore",
				"EnhancedInput",
				"UMG"
				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"DeveloperSettings",
				"Projects"
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}