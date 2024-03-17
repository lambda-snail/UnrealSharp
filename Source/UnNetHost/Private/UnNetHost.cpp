// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnNetHost.h"

#define LOCTEXT_NAMESPACE "FUnNetHostModule"

#include "Host.h"

void FUnNetHostModule::StartupModule()
{
	//const wchar_t* path = L"C:\\Projects\\Unreal\\DotnetIntegration\\Binaries\\Win64\\dummy.exe";
	// const wchar_t* path = L"C:\\Projects\\Unreal\\DotnetIntegration\\Plugins\\UnNetHost\\Resources\\dummy.exe";
	// UnNet_Execute(1, &path);
}

void FUnNetHostModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnNetHostModule, UnNetHost)