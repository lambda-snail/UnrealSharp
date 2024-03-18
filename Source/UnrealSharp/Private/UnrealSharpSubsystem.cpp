// Fill out your copyright notice in the Description page of Project Settings.


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

LambdaSnail::UnrealSharp::ActorHandle UUnrealSharpSubsystem::RegisterActorForTick(AActor* Actor) const
{
	using namespace LambdaSnail::UnrealSharp;

	ActorHandle const handle = ActorFunctions.RegisterManagedActor(
		STR("UnrealSharpCore"), STR("LambdaSnail.UnrealSharp.TestActor"));

	ActorFunctions.BindDelegates(handle, Lambda::ToFunctionPointer([Actor]()
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
	ActorFunctions.TickSingleActor(Handle, DeltaTime);
}
