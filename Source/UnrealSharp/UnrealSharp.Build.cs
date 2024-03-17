// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealSharp : ModuleRules
{
	public UnrealSharp(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		CppStandard = CppStandardVersion.Cpp20;
		
		PublicAdditionalLibraries.Add("C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/8.0.0/runtimes/win-x64/native/nethost.lib");
		//PublicAdditionalLibraries.Add("C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/8.0.0/runtimes/win-x64/native/nethost.dll");

		// PublicIncludePaths.AddRange(
		// 	new string[] {
		// 		// ... add public include paths required here ...
		// 	}
		// );
		// 		
		//
		// PrivateIncludePaths.AddRange(
		// 	new string[] {
		// 		// ... add other private include paths required here ...
		// 	}
		// );
			
		
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
				// ... add private dependencies that you statically link with here ...	
			}
		);
	}
}
