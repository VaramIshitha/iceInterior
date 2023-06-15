// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TouchInterfaceDesigner : ModuleRules
{
	public TouchInterfaceDesigner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				
			}
			);
				
		
		PrivateIncludePaths.AddRange(new string[] 
			{
				"TouchInterface/Public/Classes"
			});
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"AssetRegistry",
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
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
				//"AssetTools",
				"LevelEditor",
				"UnrealEd",
				"EditorStyle",
				"Kismet",
				"PropertyEditor",
				"Projects", //used with IPluginManager
				"TouchInterface",
				"KismetWidgets"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
				"AssetTools"
			}
			);
	}
}
