// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SolidMacros : ModuleRules
{
	public SolidMacros(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		CppStandard = CppStandardVersion.Cpp20;
		
		IWYUSupport = IWYUSupport.KeepPublicAsIsForNow;
		
		PublicIncludePaths.AddRange(
			new string[] {
				ModuleDirectory,
			}
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EngineSettings",
			}
			);
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"PropertyPath",
				}
				);
		}
	}
}
