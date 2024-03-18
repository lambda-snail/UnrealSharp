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
#else
typedef unsigned short char_t;
#endif
#else
#define CORECLR_DELEGATE_CALLTYPE
#endif

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

	typedef uint32 ActorHandle;

	typedef uint32 (CORECLR_DELEGATE_CALLTYPE *register_managed_actor_fn)(unnet_char_t const* assembly,
	                                                                   unnet_char_t const* type);
	typedef uint32 (CORECLR_DELEGATE_CALLTYPE *bind_delegates_fn)(ActorHandle, SimpleTransform (*get_transform)(ActorHandle),
	                                                           void (*set_transform)(ActorHandle, SimpleTransform));
	typedef uint32 (CORECLR_DELEGATE_CALLTYPE *tick_actors_fn)(float);
	typedef uint32 (CORECLR_DELEGATE_CALLTYPE *tick_single_actor_fn)(ActorHandle, float);

	struct ManagedActorFunctions
	{
		register_managed_actor_fn RegisterManagedActor;
		bind_delegates_fn BindDelegates;
		tick_actors_fn TickActors;
		tick_single_actor_fn TickSingleActor;
	};
	
	ManagedActorFunctions InitializeDotnetCore(FString const& root_path);
}
