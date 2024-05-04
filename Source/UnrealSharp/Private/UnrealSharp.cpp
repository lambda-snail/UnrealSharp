// Copyright Epic Games, Inc. All Rights Reserved.

#define LOCTEXT_NAMESPACE "FUnrealSharpModule"

#include "UnrealSharp.h"
#include "Logging.h"

void FUnrealSharpModule::StartupModule()
{
	//const wchar_t* path = L"C:\\Projects\\Unreal\\DotnetIntegration\\Binaries\\Win64\\dummy.exe";
	// const wchar_t* path = L"C:\\Projects\\Unreal\\DotnetIntegration\\Plugins\\UnNetHost\\Resources\\dummy.exe";
	// UnNet_Execute(1, &path);
}

void FUnrealSharpModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealSharpModule, UnrealSharp)

DEFINE_LOG_CATEGORY(UnrealSharpLog);