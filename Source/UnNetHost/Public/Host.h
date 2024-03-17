#pragma once

#ifdef _WIN32
typedef wchar_t unnet_char_t;
#else
typedef char_t unnet_char_t;
#endif

#ifdef _WIN32

#define STR(s) L ## s
#define CH(c) L ## c
#define DIR_SEPARATOR L'\\'

#else

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#define string_compare strcmp

#endif

//#include "Dotnet/coreclr_delegates.h"

// Taken from coreclr_delegates.h so that we don't need to include it in this file
// TODO: Refactor so that we don't need to expose these things in this header, which is
// the public interface to the user
#if defined(_WIN32)
#define CORECLR_DELEGATE_CALLTYPE __stdcall
#ifdef _WCHAR_T_DEFINED
typedef wchar_t char_t;
#else
typedef unsigned short char_t;
#endif
#else
#define CORECLR_DELEGATE_CALLTYPE
typedef char char_t;
#endif


using string_t = std::basic_string<char_t>;

// struct Vector
// {
// 	double X;
// 	double Y;
// 	double Z;
// };
//
// struct SimpleTransform
// {
// 	FVector3f Location;
// };
//
// typedef int (CORECLR_DELEGATE_CALLTYPE *register_managed_actor_fn)( unnet_char_t const* assembly, unnet_char_t const* type );
// typedef int (CORECLR_DELEGATE_CALLTYPE *bind_delegates_fn)(int, SimpleTransform (*get_transform)());
// typedef int (CORECLR_DELEGATE_CALLTYPE *tick_actors_fn)(float);
//
// struct ManagedActorFunctions
// {
// 	register_managed_actor_fn register_managed_actor;
// 	bind_delegates_fn bind_delegates;
// 	tick_actors_fn tick_actors;
// };

namespace LambdaSnail::UnrealSharp
{
	struct Vector
	{
		double X;
		double Y;
		double Z;
	};

	struct SimpleTransform
	{
		FVector Location;
	};

	typedef int ActorHandle;

	typedef int (CORECLR_DELEGATE_CALLTYPE *register_managed_actor_fn)(unnet_char_t const* assembly,
	                                                                   unnet_char_t const* type);
	typedef int (CORECLR_DELEGATE_CALLTYPE *bind_delegates_fn)(ActorHandle, SimpleTransform (*get_transform)(),
	                                                           void (*set_transform)(SimpleTransform));
	typedef int (CORECLR_DELEGATE_CALLTYPE *tick_actors_fn)(float);
	typedef int (CORECLR_DELEGATE_CALLTYPE *tick_single_actor_fn)(ActorHandle, float);

	struct ManagedActorFunctions
	{
		register_managed_actor_fn register_managed_actor;
		bind_delegates_fn bind_delegates;
		tick_actors_fn tick_actors;
		tick_single_actor_fn tick_single_actor;
	};

	ManagedActorFunctions UnNet_Execute(unnet_char_t const* argv);
	ManagedActorFunctions run_component_example(string_t const& root_path);
}
