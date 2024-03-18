// Based on nativehost.cpp from the dotnet samples repository: https://github.com/dotnet/samples/tree/main/core/hosting

#include "Host.h"

// Standard headers
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <functional>
#include <iostream>

#include "Dotnet/coreclr_delegates.h"
#include "Dotnet/hostfxr.h"
#include "Dotnet/nethost.h"
#include "Logging/StructuredLog.h"

#ifdef _WIN32
#include <Windows.h>

#else
#include <dlfcn.h>
#include <limits.h>

#define string_compare strcmp

#endif

namespace LambdaSnail::UnrealSharp
{
	// Globals to hold hostfxr exports
	hostfxr_initialize_for_dotnet_command_line_fn init_for_cmd_line_fptr;
	hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
	hostfxr_get_runtime_delegate_fn get_delegate_fptr;
	hostfxr_run_app_fn run_app_fptr;
	hostfxr_close_fn close_fptr;

	// Forward declarations
	bool LoadHostfxr(FString const& assembly_path);
	load_assembly_and_get_function_pointer_fn GetDotnetLoadAssembly(FString const& config_path);

	ManagedActorFunctions InitializeDotnetCore(FString const& root_path)
	{
		// Load HostFxr and get exported hosting functions
		if (!LoadHostfxr(FString()))
		{
			assert(false && "Failure: load_hostfxr()");
			return {}; // TODO: Error handling
		}
		
		// Initialize and start the dotnet core runtime
		FString const config_path = root_path + FString("DotNetLib.runtimeconfig.json");

		load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
		load_assembly_and_get_function_pointer = GetDotnetLoadAssembly(config_path);
		assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

		// ###############################
		// Initialize code 
		// ###############################

		FString const DotnetlibPath = root_path + STR("UnrealSharpCore.dll");
		FString const dotnet_type = STR("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore");
		FString const dotnet_type_method("InitActorManager");
		// Function pointer to managed delegate
		component_entry_point_fn entry_point = nullptr;
		int32_t rc = load_assembly_and_get_function_pointer(
			*DotnetlibPath, 
			*dotnet_type,
			*dotnet_type_method,
			nullptr,
			nullptr,
			reinterpret_cast<void**>(&entry_point));
		assert(rc == 0 && entry_point != nullptr && "Failure: load_assembly_and_get_function_pointer()");

		entry_point(nullptr, 0);

		// ###############################
		// Register UE LOG
		// ###############################

		typedef int (CORECLR_DELEGATE_CALLTYPE *register_unreal_logger_fn)(void (*ue_log)(TCHAR const*));
		register_unreal_logger_fn register_unreal_logger{nullptr};
		rc = load_assembly_and_get_function_pointer(
			*DotnetlibPath,
			*FString("LambdaSnail.UnrealSharp.UELog, UnrealSharpCore"),
			*FString("BindLogger"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&register_unreal_logger));
		assert(
			rc == 0 && register_unreal_logger != nullptr &&
			"Failure: Unable to register log function with managed assembly");

		register_unreal_logger([](TCHAR const* message)
		{
			UE_LOGFMT(LogTemp, Warning, "{Message}", message);
		});

		// ###############################
		// Register Actor Functions
		// ###############################

		ManagedActorFunctions ActorFunctions {};

		auto GetFunctionPointerUnmanagedCallersOnly = [&](FString const& MethodName, auto** OutFunction)-> int
		{
			return load_assembly_and_get_function_pointer(
				*DotnetlibPath,
				*FString("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"),
				*MethodName,
				UNMANAGEDCALLERSONLY_METHOD,
				nullptr,
				reinterpret_cast<void**>(OutFunction));
		};
		
		GetFunctionPointerUnmanagedCallersOnly(FString("RegisterActor"), &ActorFunctions.RegisterManagedActor);
		checkf(rc == 0 && ActorFunctions.RegisterManagedActor != nullptr, TEXT("Unable to load managed function: {FunctionName}"), TEXT("RegisterManagedActor"));
		
		GetFunctionPointerUnmanagedCallersOnly(FString("BindDelegates"), &ActorFunctions.BindDelegates);
		checkf(rc == 0 && ActorFunctions.BindDelegates != nullptr, TEXT("Unable to load managed function: {FunctionName}"), TEXT("BindDelegates"));

		// Tick Functions
		GetFunctionPointerUnmanagedCallersOnly(FString("TickActors"), &ActorFunctions.TickActors);
		checkf(rc == 0 && ActorFunctions.TickActors != nullptr, TEXT("Unable to load managed function: {FunctionName}"), TEXT("TickActors"));

		GetFunctionPointerUnmanagedCallersOnly(FString("TickSingleActor"), &ActorFunctions.TickSingleActor);
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
		init_for_cmd_line_fptr = GetExport<hostfxr_initialize_for_dotnet_command_line_fn>(
			lib, "hostfxr_initialize_for_dotnet_command_line");
		init_for_config_fptr = GetExport<hostfxr_initialize_for_runtime_config_fn>(
			lib, "hostfxr_initialize_for_runtime_config");
		get_delegate_fptr = GetExport<hostfxr_get_runtime_delegate_fn>(lib, "hostfxr_get_runtime_delegate");
		run_app_fptr = GetExport<hostfxr_run_app_fn>(lib, "hostfxr_run_app");
		close_fptr = GetExport<hostfxr_close_fn>(lib, "hostfxr_close");

		return (init_for_config_fptr && get_delegate_fptr && close_fptr);
	}

	// Load and initialize .NET Core and get desired function pointer for scenario
	load_assembly_and_get_function_pointer_fn GetDotnetLoadAssembly(FString const& ConfigPath)
	{
		// Load .NET Core
		void* load_assembly_and_get_function_pointer = nullptr;
		hostfxr_handle cxt = nullptr;
		int32_t rc = init_for_config_fptr(*ConfigPath, nullptr, &cxt);
		if (rc != 0 || cxt == nullptr)
		{
			std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
			close_fptr(cxt);
			return nullptr;
		}

		// Get the load assembly function pointer
		rc = get_delegate_fptr(
			cxt,
			hdt_load_assembly_and_get_function_pointer,
			&load_assembly_and_get_function_pointer);
		if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
			std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

		close_fptr(cxt);
		return static_cast<load_assembly_and_get_function_pointer_fn>(load_assembly_and_get_function_pointer);
	}
}
