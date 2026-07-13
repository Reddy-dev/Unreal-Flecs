// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealFlecsIris : ModuleRules
{
	public UnrealFlecsIris(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		IWYUSupport = IWYUSupport.Full;
		SetupIrisSupport(Target, true);

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"NetCore",
			"FlecsLibrary",
			"UnrealFlecs"
		});
	}
}
