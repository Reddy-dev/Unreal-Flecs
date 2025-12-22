using UnrealBuildTool;

public class UnrealFlecsTests : ModuleRules
{
    public UnrealFlecsTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PrivateIncludePaths.AddRange(
            new string[]
            {
               // ModuleDirectory,
            }
        );
            
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "StructUtils",
                "GameplayTags",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "AutomationUtils",
                "FunctionalTesting",
                "CQTest",
                "SolidMacros",
                "FlecsLibrary",
                "UnrealFlecs",
            }
        );
    }
}