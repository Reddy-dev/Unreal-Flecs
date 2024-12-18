﻿using UnrealBuildTool;

public class UnrealFlecsTests : ModuleRules
{
    public UnrealFlecsTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory
            }
        );
            
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "StructUtils",
                "SolidMacros",
                "FlecsLibrary",
                "UnrealFlecs",
                "UnrealEd",
                "AutomationUtils",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
            }
        );
    }
}