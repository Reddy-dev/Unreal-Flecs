﻿
using System.IO;
using UnrealBuildTool;

public class FlecsLibrary : ModuleRules
{
    public FlecsLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        Type = ModuleType.CPlusPlus;
        
        CppStandard = CppStandardVersion.Cpp20;
        CStandard = CStandardVersion.Latest;
        
        OptimizationLevel = OptimizationMode.Speed;
        
        // When compiling flecs.c, make sure to define flecs_EXPORTS, for example by adding -Dflecs_EXPORTS to the compiler command.
        // https://www.flecs.dev/flecs/md_docs_2Quickstart.html
        PrivateDefinitions.Add("flecs_EXPORTS");
        
        PublicDefinitions.AddRange(
            new string[]
            {
                "FLECS_CUSTOM_BUILD",
                "FLECS_CPP",
                "FLECS_MODULE",
                "FLECS_SCRIPT",
                "FLECS_SYSTEM",
                "FLECS_PIPELINE",
                "FLECS_TIMER",
                "FLECS_META",
                "FLECS_JSON",
                "FLECS_UNITS",
                "FLECS_HTTP",
                "FLECS_REST",
                //"ECS_SIMD"
            }
        );
        
        // Not Packaged
        if (Target.bBuildEditor)
        {
            PublicDefinitions.AddRange(
                new string[]
                {
                    "FLECS_STATS",
                    "FLECS_METRICS",
                    "FLECS_ALERTS",
                    "FLECS_DOC",
                    "FLECS_LOG",
                    "FLECS_PERF_TRACE",
                    "FLECS_ACCURATE_COUNTERS",
                    
                }
            );
        }
        
        PublicIncludePaths.AddRange(
            new string[] {
                ModuleDirectory + "/Public",
            }
        );
        
        PrivateIncludePaths.AddRange(
            new string[] {
                ModuleDirectory + "/Private",
                ModuleDirectory + "/Tests",
                ModuleDirectory + "/Fixtures",
            }
        );
        

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "SolidMacros",
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