using UnrealBuildTool;

public class SolidMacrosEditor : ModuleRules
{
    public SolidMacrosEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        CppStandard = CppStandardVersion.Cpp20;
        
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
            }
        );
        
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]               
                {
                    "UnrealEd",
                    "PropertyEditor",
                    "ToolWidgets",
                    "GraphEditor",
                    "BlueprintGraph",
                }
            );
        }
        
        CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;
    }
}
