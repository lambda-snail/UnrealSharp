﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealSharpSubsystem.h"

#include <minwindef.h>

#include "Host.h"
#include "LambdaHelpers.h"
#include "Logging/StructuredLog.h"


void UUnrealSharpSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	using namespace LambdaSnail::UnrealSharp;

	FString const Path ("C:\\Projects\\Unreal\\DotnetIntegration\\Plugins\\UnrealSharp\\Resources\\");
	ActorFunctions = InitializeDotnetCore(Path);
}

void UUnrealSharpSubsystem::Deinitialize()
{
}

LambdaSnail::UnrealSharp::ActorHandle UUnrealSharpSubsystem::RegisterActorForTick(AActor* Actor)
{
	using namespace LambdaSnail::UnrealSharp;

	ActorHandle const Handle = ActorFunctions.RegisterManagedActor(
		STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.TestActor"));

	RegisteredActors.Emplace(Handle, Actor); // TODO: Error handling if actor is already registered?
	
	ActorFunctions.BindDelegates(Handle, Lambda::ToFunctionPointer([this](ActorHandle ActorHandle)
	    {
	        UE_LOGFMT(LogTemp, Warning, "GetTransform for handle {Handle}", ActorHandle);
	        SimpleTransform const Transform{this->RegisteredActors[ActorHandle]->GetActorLocation()};
	        return Transform;
	    }),
Lambda::ToFunctionPointer([this](ActorHandle ActorHandle, SimpleTransform Transform)
	    {
			UE_LOGFMT(LogTemp, Warning, "SetTransform for handle {Handle}", ActorHandle);
	        this->RegisteredActors[ActorHandle]->SetActorLocation(Transform.Location);
	    }
	));

	return Handle;
}

void UUnrealSharpSubsystem::TickActor(LambdaSnail::UnrealSharp::ActorHandle Handle, float DeltaTime)
{
	ActorFunctions.TickSingleActor(Handle, DeltaTime);
}
