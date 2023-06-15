using UnrealBuildTool;

public class VirtualShapeDesigner : ModuleRules
{
    public VirtualShapeDesigner(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[] {
				
            }
        );
				
		
        PrivateIncludePaths.AddRange(new string[] 
        {
            "VirtualShapeDesigner/Private/Assets",
            "VirtualShapeDesigner/Private/DetailsTab",
            "VirtualShapeDesigner/Private/Editor",
            "VirtualShapeDesigner/Private/Tools",
            "VirtualShapeDesigner/Private/Viewport"
        });
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "TouchInterfaceDesigner"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "InputCore",
                "TouchInterface",
                "KismetWidgets",
                "PropertyEditor",
                "Projects",
                "UnrealEd",
                "DeveloperSettings",
                "EditorStyle"
            }
        );
    }
}