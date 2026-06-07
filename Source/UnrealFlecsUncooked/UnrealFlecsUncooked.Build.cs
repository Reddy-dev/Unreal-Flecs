using UnrealBuildTool;

public class UnrealFlecsUncooked : ModuleRules
{
    public UnrealFlecsUncooked(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
                "UnrealFlecs",
                "KismetCompiler",
                "UnrealEd",
            }
        );
        
        CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;
    }
}