// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealFlecsGameFramework : ModuleRules
{
	public UnrealFlecsGameFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		CppStandard = CppStandardVersion.Cpp23;
		IWYUSupport = IWYUSupport.Full;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"SolidMacros",
				"FlecsLibrary",
				"UnrealFlecs",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealFlecsNetworking",
			}
		);

		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;
	}
}
