// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealSharpSubsystem.h"

#include <minwindef.h>

#include "Host.h"
#include "Logging/StructuredLog.h"
#include "UObject/UnrealTypePrivate.h"


void UUnrealSharpSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	using namespace LambdaSnail::UnrealSharp;

	FString const Path = FPaths::Combine (FPaths::ProjectDir(), "Plugins", "UnrealSharp", "Resources");
	ActorFunctions = InitializeDotnetCore(Path);
}

void UUnrealSharpSubsystem::Deinitialize()
{
}

LambdaSnail::UnrealSharp::ActorHandle UUnrealSharpSubsystem::RegisterActorForTick(AActor* Actor)
{
	using namespace LambdaSnail::UnrealSharp;

	ActorHandle const Handle = ActorFunctions.RegisterManagedActor(
		STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.TestActor"), Actor);

	RegisteredActors.Emplace(Handle, Actor); // TODO: Error handling if actor is already registered?
	
	return Handle;
}

void UUnrealSharpSubsystem::TickActor(LambdaSnail::UnrealSharp::ActorHandle Handle, float DeltaTime)
{
	ActorFunctions.TickSingleActor(Handle, DeltaTime);
}
