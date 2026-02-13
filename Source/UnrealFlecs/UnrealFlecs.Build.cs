// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealFlecs : ModuleRules
{
	public UnrealFlecs(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		CppStandard = CppStandardVersion.Cpp20;
		IWYUSupport = IWYUSupport.Full;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"FlecsLibrary",
				"SolidMacros",
				"GameplayTags",
				"AssetRegistry",
				"StructUtils",
				"NetCore",
				"NetCommon",
				"PhysicsCore",
                "Chaos",
                "ChaosCore",
                "ChaosSolverEngine",
                "DeveloperSettings",
                "ModularGameplay",
                "GameFeatures"
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
