using UnrealBuildTool;

public class SolidMacrosEditor : ModuleRules
{
    public SolidMacrosEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        IWYUSupport = IWYUSupport.Full;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "SolidMacros",
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
                "Kismet",
                "UnrealEd",
                "PropertyEditor",
            }
        );
        
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("ToolWidgets");
        }
    }
}