using UnrealBuildTool;

public class UnrealFlecsEditor : ModuleRules
{
    public UnrealFlecsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        IWYUSupport = IWYUSupport.Full;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects",
                "UnrealFlecs",
                "FlecsLibrary",
                "StructUtils",
                "InputCore",
                "AssetRegistry",
                "SolidMacros",
                "UnrealEd",
                "ToolMenus",
                "EditorStyle",
                "PropertyEditor",
                "GraphEditor",
                "BlueprintGraph",
                "StructUtilsEditor",
            }
        );
    }
}