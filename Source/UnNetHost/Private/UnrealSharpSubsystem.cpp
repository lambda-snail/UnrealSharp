// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealSharpSubsystem.h"

#include <minwindef.h>

#include "Host.h"
#include "Logging/StructuredLog.h"


template <class F>
struct lambda_traits : lambda_traits<decltype(&F::operator())>
{ };

template <typename F, typename R, typename... Args>
struct lambda_traits<R(F::*)(Args...)> : lambda_traits<R(F::*)(Args...) const>
{ };

template <class F, class R, class... Args>
struct lambda_traits<R(F::*)(Args...) const> {
	using pointer = typename std::add_pointer<R(Args...)>::type;

	static pointer const cify(F&& f) {
		static F fn = std::forward<F>(f);
		return [](Args... args) {
			return fn(std::forward<Args>(args)...);
		};
	}
};

template <class F>
inline typename lambda_traits<F>::pointer cify(F&& f) {
	return lambda_traits<F>::cify(std::forward<F>(f));
}


void UUnrealSharpSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	using namespace LambdaSnail::UnrealSharp;
	
	const wchar_t* path = L"C:\\Projects\\Unreal\\DotnetIntegration\\Plugins\\UnNetHost\\Resources\\";
	ActorFunctions = UnNet_Execute(path);
}

void UUnrealSharpSubsystem::Deinitialize()
{
	
}

LambdaSnail::UnrealSharp::ActorHandle UUnrealSharpSubsystem::RegisterActorForTick(AActor* Actor) const
{
	using namespace LambdaSnail::UnrealSharp;
	
	ActorHandle const handle = ActorFunctions.register_managed_actor(STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.TestActor"));

	ActorFunctions.bind_delegates(handle, cify([Actor]()
	{
		UE_LOGFMT(LogTemp, Warning, "GetTransform for handle {Handle}", 1);
		SimpleTransform const Transform { Actor->GetActorLocation() };
		return Transform;
	}),
	cify([Actor](SimpleTransform Transform)
	{
		Actor->SetActorLocation(Transform.Location);
	}
	));

	return handle;
}

void UUnrealSharpSubsystem::TickActor(LambdaSnail::UnrealSharp::ActorHandle Handle, float DeltaTime)
{
	ActorFunctions.tick_single_actor(Handle, DeltaTime);
}
