﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealSharpSubsystem.h"

#include <minwindef.h>

#include "Host.h"
#include "LambdaHelpers.h"
#include "Logging/StructuredLog.h"


void UUnrealSharpSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	using namespace LambdaSnail::UnrealSharp;

	const wchar_t* path = L"C:\\Projects\\Unreal\\DotnetIntegration\\Plugins\\UnrealSharp\\Resources\\";
	ActorFunctions = UnNet_Execute(path);
}

void UUnrealSharpSubsystem::Deinitialize()
{
}

LambdaSnail::UnrealSharp::ActorHandle UUnrealSharpSubsystem::RegisterActorForTick(AActor* Actor) const
{
	using namespace LambdaSnail::UnrealSharp;

	ActorHandle const handle = ActorFunctions.register_managed_actor(
		STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.TestActor"));

	ActorFunctions.bind_delegates(handle, Lambda::ToFunctionPointer([Actor]()
	    {
	        UE_LOGFMT(LogTemp, Warning, "GetTransform for handle {Handle}", 1);
	        SimpleTransform const Transform{Actor->GetActorLocation()};
	        return Transform;
	    }),
Lambda::ToFunctionPointer([Actor](SimpleTransform Transform)
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
