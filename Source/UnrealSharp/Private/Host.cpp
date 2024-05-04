// Based on nativehost.cpp from the dotnet samples repository: https://github.com/dotnet/samples/tree/main/core/hosting

#include "Host.h"

// Standard headers
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "Dotnet/CoreclrDelegates.h"
#include "Dotnet/HostFxr.h"
#include "Dotnet/Nethost.h"
#include "Logging.h"

#ifdef _WIN32
#include <Windows.h>

#else
#include <dlfcn.h>
#include <limits.h>

#define string_compare strcmp

#endif



// // Store the module handle
// HMODULE g_hModule = nullptr;
//
// BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
// 	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
// 		g_hModule = hModule;
// 	}
// 	return TRUE;
// }
//
// // Get the DLL filename
// std::wstring GetDllFilename() {
// 	wchar_t buf[MAX_PATH];
// 	GetModuleFileName(g_hModule, buf, MAX_PATH);
// 	return buf;
// }


namespace LambdaSnail::UnrealSharp
{
	// Globals to hold hostfxr exports
	HostfxrInitializeForDotnetCommandLine_Fn InitForCmdLine_Fptr;
	HostFxrInitializeForRuntimeConfig_Fn InitForConfig_Fptr;
	HostFxrGetRuntimeDelegate_Fn GetDelegate_Fptr;
	HostFxrRunApp_Fn RunApp_Fptr;
	HostFxrClose_Fn Close_Fptr;

	// Forward declarations
	bool LoadHostfxr(FString const& assembly_path);
	LoadAssemblyAndGetFunctionPointer_Fn GetDotnetLoadAssembly(FString const& config_path);

	ManagedActorFunctions InitializeDotnetCore(FString const& RootPath)
	{
		// Load HostFxr and get exported hosting functions
		if (!LoadHostfxr(FString()))
		{
			assert(false && "Failure: load_hostfxr()");
			return {}; // TODO: Error handling
		}
		
		// Initialize and start the dotnet core runtime
		FString const ConfigPath = FPaths::Combine(RootPath, "DotNetLib.runtimeconfig.json");

		LoadAssemblyAndGetFunctionPointer_Fn LoadAssemblyAndGetFunctionPointer = nullptr;
		LoadAssemblyAndGetFunctionPointer = GetDotnetLoadAssembly(ConfigPath);
		assert(LoadAssemblyAndGetFunctionPointer != nullptr && "Failure: get_dotnet_load_assembly()");

		// ###############################
		// Initialize Actor Manager 
		// ###############################

		FString const DotnetlibPath = FPaths::Combine(RootPath, "UnrealSharpCore.dll");
		FString const DotnetType = STR("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore");
		FString const DotnetTypeMethod("InitActorManager");
		// Function pointer to managed delegate
		ComponentEntryPoint_Fn EntryPoint = nullptr;
		int32_t rc = LoadAssemblyAndGetFunctionPointer(
			*DotnetlibPath, 
			*DotnetType,
			*DotnetTypeMethod,
			nullptr,
			nullptr,
			reinterpret_cast<void**>(&EntryPoint));
		assert(rc == 0 && entry_point != nullptr && "Failure: load_assembly_and_get_function_pointer()");
		
		EntryPoint(nullptr, 0);

		// ###############################
		// Register UE LOG
		// ###############################
		auto GetFunctionPointerUnmanagedCallersOnly = [&](FString const& MethodName, auto** OutFunction, FString const& AssemblyQualifiedType = FString("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"))-> int32
		{
			return LoadAssemblyAndGetFunctionPointer(
				*DotnetlibPath,
				*FString(AssemblyQualifiedType),
				*MethodName,
				UNMANAGEDCALLERSONLY_METHOD,
				nullptr,
				reinterpret_cast<void**>(OutFunction));
		};
		

		typedef int (CORECLR_DELEGATE_CALLTYPE *RegisterUnrealLogger_Fn)(void (*ue_log)(TCHAR const*));
		RegisterUnrealLogger_Fn RegisterUnrealManagedLogger { nullptr };
		rc = GetFunctionPointerUnmanagedCallersOnly(FString("BindLogger"), &RegisterUnrealManagedLogger, FString("LambdaSnail.UnrealSharp.UELog, UnrealSharpCore"));
		checkf(rc == 0 && RegisterUnrealManagedLogger != nullptr, TEXT("Unable to load managed function: {FunctionName}"), TEXT("register_unreal_logger"));

		RegisterUnrealManagedLogger([](TCHAR const* Message)
		{
			UE_LOGFMT(LogTemp, Warning, "{Message}", Message);
		});

		// ###############################
		// Register Actor Functions
		// ###############################

		ManagedActorFunctions ActorFunctions {};
		
		rc = GetFunctionPointerUnmanagedCallersOnly(FString("RegisterActor"), &ActorFunctions.RegisterManagedActor);
		checkf(rc == 0 && ActorFunctions.RegisterManagedActor != nullptr, TEXT("Unable to load managed function: {FunctionName}"), TEXT("RegisterManagedActor"));
		
		// Tick Functions
		rc = GetFunctionPointerUnmanagedCallersOnly(FString("TickActors"), &ActorFunctions.TickActors);
		checkf(rc == 0 && ActorFunctions.TickActors != nullptr, TEXT("Unable to load managed function: {FunctionName}"), TEXT("TickActors"));

		rc = GetFunctionPointerUnmanagedCallersOnly(FString("TickSingleActor"), &ActorFunctions.TickSingleActor);
		checkf(rc == 0 && ActorFunctions.TickSingleActor != nullptr, TEXT("Unable to load managed function: {FunctionName}"), TEXT("TickSingleActor"));
		
		return ActorFunctions;
	}

	/********************************************************************************************
	 * Function used to load and activate .NET Core
	 ********************************************************************************************/

	void* LoadLibrary(TCHAR const* path)
	{
#ifdef _WIN32
		HMODULE h = ::LoadLibraryW(path);
		assert(h != nullptr);
		return (void*)h;
#else
		void *h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		assert(h != nullptr);
		return h;
#endif
	}

	template <typename T>
	T GetExport(void* h, char const* name)
	{
		static_assert(std::is_pointer_v<T>);

#ifdef _WIN32
		void* f = ::GetProcAddress(static_cast<HMODULE>(h), name);
#else
        void *f = dlsym(h, name);
#endif
		assert(f != nullptr);
		return static_cast<T>(f);
	}
	
	// Using the nethost library, discover the location of hostfxr and get exports
	bool LoadHostfxr(FString const& AssemblyPath)
	{
		get_hostfxr_parameters params{sizeof(get_hostfxr_parameters), *AssemblyPath, nullptr};
		// Pre-allocate a large buffer for the path to hostfxr
		TCHAR buffer[MAX_PATH];
		size_t buffer_size = sizeof(buffer) / sizeof(TCHAR);
		int32_t const rc = get_hostfxr_path(buffer, &buffer_size, &params);
		if (rc != 0)
			return false;

		// Load hostfxr and get desired exports
		void* lib = LoadLibrary(buffer);
		InitForCmdLine_Fptr = GetExport<HostfxrInitializeForDotnetCommandLine_Fn>(
			lib, "hostfxr_initialize_for_dotnet_command_line");
		InitForConfig_Fptr = GetExport<HostFxrInitializeForRuntimeConfig_Fn>(
			lib, "hostfxr_initialize_for_runtime_config");
		GetDelegate_Fptr = GetExport<HostFxrGetRuntimeDelegate_Fn>(lib, "hostfxr_get_runtime_delegate");
		RunApp_Fptr = GetExport<HostFxrRunApp_Fn>(lib, "hostfxr_run_app");
		Close_Fptr = GetExport<HostFxrClose_Fn>(lib, "hostfxr_close");

		return (InitForConfig_Fptr && GetDelegate_Fptr && Close_Fptr);
	}

	// Load and initialize .NET Core and get desired function pointer for scenario
	LoadAssemblyAndGetFunctionPointer_Fn GetDotnetLoadAssembly(FString const& ConfigPath)
	{
		// Load .NET Core
		void* load_assembly_and_get_function_pointer = nullptr;
		hostfxr_handle cxt = nullptr;
		int32_t rc = InitForConfig_Fptr(*ConfigPath, nullptr, &cxt);
		if (rc != 0 || cxt == nullptr)
		{
			UE_LOGFMT(UnrealSharpLog, Error, "Init dotnet core failed: {ReturnCode}", rc);
			Close_Fptr(cxt);
			return nullptr;
		}

		// Get the load assembly function pointer
		rc = GetDelegate_Fptr(
			cxt,
			hdt_load_assembly_and_get_function_pointer,
			&load_assembly_and_get_function_pointer);
		if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
			std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

		Close_Fptr(cxt);
		return static_cast<LoadAssemblyAndGetFunctionPointer_Fn>(load_assembly_and_get_function_pointer);
	}
}
