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

	ManagedActorFunctions UnNet_Execute(FString const& argv)
	{
		return InitializeDotnetCore(argv);
	}

	ManagedActorFunctions InitializeDotnetCore(FString const& root_path)
	{
		//
		// STEP 1: Load HostFxr and get exported hosting functions
		//
		if (!LoadHostfxr(FString()))
		{
			assert(false && "Failure: load_hostfxr()");
			return {}; // TODO: Error handling
		}

		//
		// STEP 2: Initialize and start the .NET Core runtime
		//
		FString const config_path = root_path + FString("DotNetLib.runtimeconfig.json");

		load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
		load_assembly_and_get_function_pointer = GetDotnetLoadAssembly(config_path);
		assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

		// ###############################
		// Initialize code 
		// ###############################

		FString const dotnetlib_path = root_path + STR("UnrealSharpCore.dll");
		FString const dotnet_type = STR("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore");
		FString const dotnet_type_method("InitActorManager");
		// Function pointer to managed delegate
		component_entry_point_fn entry_point = nullptr;
		int32_t rc = load_assembly_and_get_function_pointer(
			*dotnetlib_path,
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
			*dotnetlib_path,
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
		// Register Test Actor
		// ###############################
		
		register_managed_actor_fn register_managed_actor{nullptr};
		rc = load_assembly_and_get_function_pointer(
			*dotnetlib_path,
			*FString("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"),
			*FString("RegisterActor"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&register_managed_actor));
		assert(rc == 0 && register_managed_actor != nullptr && "Failure: load_assembly_and_get_function_pointer()");

		int handle1 = register_managed_actor(STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.TestActor"));
		UE_LOGFMT(LogTemp, Warning, "Created managed actor with handle: {Handle}", handle1);
		
		// Binding Delegates
		
		bind_delegates_fn bind_delegates{nullptr};
		rc = load_assembly_and_get_function_pointer(
			*dotnetlib_path,
			*FString("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"),
			*FString("BindDelegates"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&bind_delegates));
		assert(rc == 0 && bind_delegates != nullptr && "Failure: Could not load 'BindDelegates'");

		bind_delegates(handle1, []()
		{
			UE_LOGFMT(LogTemp, Warning, "GetTransform for handle {Handle}", 1);
			SimpleTransform const Transform{FVector(1, .4, -1)};
			//SimpleTransform const Transform{5, 2, -1.976};
			return Transform;
		},
		[](SimpleTransform Transform)
		{
			UE_LOGFMT(LogTemp, Warning, "SetTransform({Input}) for handle {Handle}", Transform.Location.ToCompactString(), 1);
		}
		);

		// Call Tick
		tick_actors_fn tick_actors{nullptr};
		rc = load_assembly_and_get_function_pointer(
			*dotnetlib_path,
			*FString("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"),
			*FString("TickActors"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&tick_actors));
		assert(rc == 0 && tick_actors != nullptr && "Failure: Could not load 'TickActors'");

		tick_single_actor_fn tick_single_actor { nullptr };
		rc = load_assembly_and_get_function_pointer(
			*dotnetlib_path,
			*FString("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"),
			*FString("TickSingleActor"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&tick_single_actor));
		assert(rc == 0 && tick_single_actor != nullptr && "Failure: Could not load 'TickSingleActor'");
		
		tick_actors(0.1);

		ManagedActorFunctions ActorFunctions
		{
			.register_managed_actor = register_managed_actor,
			.bind_delegates = bind_delegates,
			.tick_actors = tick_actors,
			.tick_single_actor = tick_single_actor
		};

		return ActorFunctions;
	}

	/********************************************************************************************
	 * Function used to load and activate .NET Core
	 ********************************************************************************************/

	// Forward declarations
	void* load_library(TCHAR const*);

	template <typename T>
	T get_export(void*, char const*);


	void* load_library(TCHAR const* path)
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
	T get_export(void* h, char const* name)
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
		void* lib = load_library(buffer);
		init_for_cmd_line_fptr = get_export<hostfxr_initialize_for_dotnet_command_line_fn>(
			lib, "hostfxr_initialize_for_dotnet_command_line");
		init_for_config_fptr = get_export<hostfxr_initialize_for_runtime_config_fn>(
			lib, "hostfxr_initialize_for_runtime_config");
		get_delegate_fptr = get_export<hostfxr_get_runtime_delegate_fn>(lib, "hostfxr_get_runtime_delegate");
		run_app_fptr = get_export<hostfxr_run_app_fn>(lib, "hostfxr_run_app");
		close_fptr = get_export<hostfxr_close_fn>(lib, "hostfxr_close");

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
