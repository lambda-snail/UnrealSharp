// Based on nativehost.cpp from the dotnet samples repository: https://github.com/dotnet/samples/tree/main/core/hosting

#include "Host.h"

// Standard headers
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "Dotnet/coreclr_delegates.h"
#include "Dotnet/hostfxr.h"
#include "Dotnet/nethost.h"
#include "Logging/StructuredLog.h"

#ifdef _WIN32
#include <Windows.h>

#define STR(s) L ## s
#define CH(c) L ## c
#define DIR_SEPARATOR L'\\'

#else
#include <dlfcn.h>
#include <limits.h>

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#define string_compare strcmp

#endif

using string_t = std::basic_string<char_t>;

namespace
{
	// Globals to hold hostfxr exports
	hostfxr_initialize_for_dotnet_command_line_fn init_for_cmd_line_fptr;
	hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
	hostfxr_get_runtime_delegate_fn get_delegate_fptr;
	hostfxr_run_app_fn run_app_fptr;
	hostfxr_close_fn close_fptr;

	// Forward declarations
	bool load_hostfxr(char_t const* assembly_path);
	load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(char_t const* config_path);

	int32_t run_component_example(string_t const& root_path);
}

#if defined(_WIN32)
int32_t __cdecl wmain(int32_t argc, wchar_t* argv[])
#else
int32_t main(int32_t argc, char *argv[])
#endif
{
	// Get the current executable's directory
	// This sample assumes the managed assembly to load and its runtime configuration file are next to the host
	char_t host_path[MAX_PATH];
#if _WIN32
	auto const size = ::GetFullPathNameW(argv[0], sizeof(host_path) / sizeof(char_t), host_path, nullptr);
	assert(size != 0);
#else
    auto resolved = realpath(argv[0], host_path);
    assert(resolved != nullptr);
#endif

	string_t root_path = host_path;
	auto const pos = root_path.find_last_of(DIR_SEPARATOR);
	assert(pos != string_t::npos);
	root_path = root_path.substr(0, pos + 1);


	return run_component_example(root_path);
}

int UnNet_Execute(int argc, unnet_char_t const* argv[])
{
	// Get the current executable's directory
	// This sample assumes the managed assembly to load and its runtime configuration file are next to the host
	char_t host_path[MAX_PATH];
#if _WIN32
	auto size = ::GetFullPathNameW(argv[0], sizeof(host_path) / sizeof(char_t), host_path, nullptr);
	assert(size != 0);
#else
    auto resolved = realpath(argv[0], host_path);
    assert(resolved != nullptr);
#endif

	string_t root_path = host_path;
	auto pos = root_path.find_last_of(DIR_SEPARATOR);
	assert(pos != string_t::npos);
	root_path = root_path.substr(0, pos + 1);

	return run_component_example(root_path);
}

namespace
{
	void test_fn()
	{
		std::cout << "[C++] In c++ again!" << std::endl;
	}

	// So we can pass a lambda or a function ptr to a free c++ function for consumption
	// by the hosted dotnet code
	// However, we cannot use std::function it appears (which would be prefferable) ...
	//
	// The signature of the method in dotnet will look like this (no return, no params):
	//         [UnmanagedCallersOnly]
	//         public static unsafe void TestFnPtr(delegate*<void> fn_from_cpp)
	// and it can be called like this: 
	//         fn_from_cpp();
	void pass_fnptr_to_dotnet(load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer,
	                          string_t const& dotnetlib_path, char_t const* dotnet_type, int& rc)
	{
		typedef void (CORECLR_DELEGATE_CALLTYPE *send_callback_to_dotnet_fn)(void (*fn)());
		send_callback_to_dotnet_fn callback;
		rc = load_assembly_and_get_function_pointer(
			dotnetlib_path.c_str(),
			dotnet_type,
			STR("TestFnPtr"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			(void**)&callback);
		assert(callback && "Unable to load function TestFnPtr");

		// We can use a pointer to a function defined elsewhere 
		callback(&test_fn);

		// but the dotnet code can also consume a lambda function
		// auto fn = []()
		// {
		//     std::cout << "C++ lambda called from dotnet!" << std::endl;
		// }; 
		// callback(fn);

		// However, std::function cannot be used here as far as I know
		// std::function fn = ...
		//callback(&fn.target); // Does not work
	}

	double test_fn_arumgents_and_returns(int32_t i)
	{
		std::cout << "[C++] Recieved " << i << " from dotnet!" << std::endl;
		return static_cast<double_t>(i) + .5;
	}

	void pass_fnptr_to_dotnet_witharguments(
		load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer, string_t const dotnetlib_path,
		char_t const* dotnet_type, int& rc)
	{
		typedef void (CORECLR_DELEGATE_CALLTYPE *send_callback_to_dotnet_fn)(double_t (*fn)(int32_t));
		send_callback_to_dotnet_fn callback;
		rc = load_assembly_and_get_function_pointer(
			dotnetlib_path.c_str(),
			dotnet_type,
			STR("TestFnPtrWithArgs"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			(void**)&callback);
		assert(callback && "Unable to load function TestFnPtrWithArgs");

		// We can use a pointer to a function defined elsewhere 
		// callback(&test_fn_arumgents_and_returns);

		auto fn = [](int32_t i) -> double_t
		{
			std::cout << "[C++] A lambda recieved " << i << " from dotnet!" << std::endl;
			return static_cast<double_t>(i) + 3.14;
		};
		callback(fn);
	}

	// Example of how to send and receive a string to/from c#.
	// The corresponding function in c# has the following signature:
	//      public static unsafe void TestStringInputOutput(delegate* unmanaged<IntPtr, IntPtr> str_fn)
	//
	// THe IntPtr should be Marshalled differently depending on if you are on Linux or Windows, as the character types
	// may have different sizes on different systems. See https://learn.microsoft.com/en-us/dotnet/standard/native-interop/charset. 
	void pass_fnptr_with_strings(load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer,
	                             string_t const dotnetlib_path, char_t const* dotnet_type, int& rc)
	{
		typedef void (CORECLR_DELEGATE_CALLTYPE *send_callback_to_dotnet_fn)(wchar_t const*(*fn)(wchar_t const*));
		send_callback_to_dotnet_fn callback;
		rc = load_assembly_and_get_function_pointer(
			dotnetlib_path.c_str(),
			dotnet_type,
			STR("TestStringInputOutput"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			(void**)&callback);
		assert(callback && "Unable to load function TestStringInputOutput");

		callback([](wchar_t const* str) -> wchar_t const* {
			std::wcout << STR("[C++] C# sent the following string: ") << str << std::endl;
			return STR("This string is from c++ :)");
		});
	}

	int32_t run_component_example(string_t const& root_path)
	{
		//
		// STEP 1: Load HostFxr and get exported hosting functions
		//
		if (!load_hostfxr(nullptr))
		{
			assert(false && "Failure: load_hostfxr()");
			return EXIT_FAILURE;
		}

		//
		// STEP 2: Initialize and start the .NET Core runtime
		//
		const string_t config_path = root_path + STR("DotNetLib.runtimeconfig.json");
		wprintf(L"Config Path: %25s\n", config_path.c_str());

		load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
		load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
		assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

		//
		// STEP 3: Load managed assembly and get function pointer to a managed method
		//
		// string_t const dotnetlib_path = root_path + STR("DotNetLib.dll");
		// char_t const *dotnet_type = STR("LambdaSnail.UnrealSharp.ActorManager, LambdaSnail.UnrealSharp");
		// char_t const *dotnet_type_method = STR("InitActorManager");
		// // Function pointer to managed delegate
		// //component_entry_point_fn hello = nullptr;
		// typedef void (CORECLR_DELEGATE_CALLTYPE *entry_point_fn)();
		// entry_point_fn entry_point {nullptr};
		//
		// int32_t rc = load_assembly_and_get_function_pointer(
		//     dotnetlib_path.c_str(),
		//     dotnet_type,
		//     dotnet_type_method,
		//     UNMANAGEDCALLERSONLY_METHOD,
		//     nullptr,
		//     (void**)&entry_point);
		// assert(rc == 0 && hello != nullptr && "Failure: load_assembly_and_get_function_pointer()");

		// ###############################
		// Initialize code 
		// ###############################

		string_t const dotnetlib_path = root_path + STR("UnrealSharpCore.dll");
		char_t const* dotnet_type = STR("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore");
		char_t const* dotnet_type_method = STR("InitActorManager");
		// Function pointer to managed delegate
		component_entry_point_fn entry_point = nullptr;
		int32_t rc = load_assembly_and_get_function_pointer(
			dotnetlib_path.c_str(),
			dotnet_type,
			dotnet_type_method,
			nullptr,
			nullptr,
			reinterpret_cast<void**>(&entry_point));
		assert(rc == 0 && hello != nullptr && "Failure: load_assembly_and_get_function_pointer()");

		entry_point(nullptr, 0);

		// ###############################
		// Register UE LOG
		// ###############################

		typedef int (CORECLR_DELEGATE_CALLTYPE *register_unreal_logger_fn)( void(*ue_log)(unnet_char_t const*) );
		register_unreal_logger_fn register_unreal_logger { nullptr };
		rc = load_assembly_and_get_function_pointer(
					dotnetlib_path.c_str(),
					STR("LambdaSnail.UnrealSharp.UELog, UnrealSharpCore"),
					STR("BindLogger"),
					UNMANAGEDCALLERSONLY_METHOD,
					nullptr,
					reinterpret_cast<void**>(&register_unreal_logger));
		assert(rc == 0 && register_unreal_logger != nullptr && "Failure: Unable to register log function with managed assembly");

		register_unreal_logger([](unnet_char_t const* message)
		{
			UE_LOGFMT(LogTemp, Warning, "{Message}", message);
		});
		
		// ###############################
		// Register Test Actor
		// ###############################

		typedef int (CORECLR_DELEGATE_CALLTYPE *register_managed_actor_fn)(
			unnet_char_t const* assembly, unnet_char_t const* type);
		register_managed_actor_fn register_managed_actor{nullptr};
		rc = load_assembly_and_get_function_pointer(
			dotnetlib_path.c_str(),
			STR("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"),
			STR("RegisterActor"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&register_managed_actor));
		assert(rc == 0 && register_managed_actor != nullptr && "Failure: load_assembly_and_get_function_pointer()");

		int handle1 = register_managed_actor(STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.SomeActor"));
		UE_LOGFMT(LogTemp, Warning, "Created managed actor with handle: {Handle}", handle1);

		int handle2 = register_managed_actor(STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.SomeActor"));
		UE_LOGFMT(LogTemp, Warning, "Created managed actor with handle: {Handle}", handle2);

		int handle3 = register_managed_actor(STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.SomeActor"));
		UE_LOGFMT(LogTemp, Warning, "Created managed actor with handle: {Handle}", handle3);

		struct Vector
		{
			double X;
			double Y;
			double Z;
		};
		
		// Binding Delegates
		struct SimpleTransform
		{
			FVector3f Location;
			// double X;
			// double Y;
			// double Z;
		};

		typedef int (CORECLR_DELEGATE_CALLTYPE *bind_delegates_fn)(int, SimpleTransform (*get_transform)());
		bind_delegates_fn bind_delegates{nullptr};
		rc = load_assembly_and_get_function_pointer(
			dotnetlib_path.c_str(),
			STR("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"),
			STR("BindDelegates"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&bind_delegates));
		assert(rc == 0 && bind_delegates != nullptr && "Failure: Could not load 'BindDelegates'");

		bind_delegates(handle1, []()
		{
			UE_LOGFMT(LogTemp, Warning, "GetTransform for handle {Handle}", 1);
			SimpleTransform const Transform{FVector3f(0.f, 0.f, 0.f)};
			//SimpleTransform const Transform{0.4, 0.1, 0};
			return Transform;
		});

		bind_delegates(handle2, []()
		{
			UE_LOGFMT(LogTemp, Warning, "GetTransform for handle {Handle}", 2);
			SimpleTransform const Transform{FVector3f(1.f, .4f, -1.f)};
			//SimpleTransform const Transform{5, 2, -1.976};
			return Transform;
		});

		bind_delegates(handle3, []()
		{
			UE_LOGFMT(LogTemp, Warning, "GetTransform for handle {Handle}", 3);
			SimpleTransform const Transform{FVector3f(-5.f, 2.f, 1.f)};
			//SimpleTransform const Transform{43, 99, -1};
			return Transform;
		});

		// Call Tick
		typedef int (CORECLR_DELEGATE_CALLTYPE *tick_actors_fn)(float);
		tick_actors_fn tick_actors { nullptr };
		rc = load_assembly_and_get_function_pointer(
			dotnetlib_path.c_str(),
			STR("LambdaSnail.UnrealSharp.ActorManager, UnrealSharpCore"),
			STR("TickActors"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&tick_actors));
		assert(rc == 0 && tick_actors != nullptr && "Failure: Could not load 'BindDelegates'");

		tick_actors(0.1);
		

		return EXIT_SUCCESS;
	}
}

/********************************************************************************************
 * Function used to load and activate .NET Core
 ********************************************************************************************/

namespace
{
	// Forward declarations
	void* load_library(char_t const*);

	template <typename T>
	T get_export(void*, char const*);

#ifdef _WIN32
	void* load_library(char_t const* path)
	{
		HMODULE h = ::LoadLibraryW(path);
		assert(h != nullptr);
		return (void*)h;
	}

#else
    void *load_library(char_t const* path)
    {
        void *h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        assert(h != nullptr);
        return h;
    }
#endif

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
	bool load_hostfxr(char_t const* assembly_path)
	{
		get_hostfxr_parameters params{sizeof(get_hostfxr_parameters), assembly_path, nullptr};
		// Pre-allocate a large buffer for the path to hostfxr
		char_t buffer[MAX_PATH];
		size_t buffer_size = sizeof(buffer) / sizeof(char_t);
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
	load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(char_t const* config_path)
	{
		// Load .NET Core
		void* load_assembly_and_get_function_pointer = nullptr;
		hostfxr_handle cxt = nullptr;
		int32_t rc = init_for_config_fptr(config_path, nullptr, &cxt);
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
