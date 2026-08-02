// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealFlecsNetworking : ModuleRules
{
	public UnrealFlecsNetworking(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		CppStandard = CppStandardVersion.Cpp20;
		IWYUSupport = IWYUSupport.Full;

		SetupIrisSupport(Target, true);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealFlecs",
				"FlecsLibrary",
				"SolidMacros",
				"NetCore",
			}
		);

		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;
	}
}
