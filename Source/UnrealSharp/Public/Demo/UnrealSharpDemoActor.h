// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Host.h"
#include "GameFramework/Actor.h"
#include "UnrealSharpDemoActor.generated.h"

class UUnrealSharpSubsystem;

/**
 * Demonstrates how to register an actor with the UnrealSharpSubsystem to obtain an actor handle, and how to
 * invoke c# logic on each tick.
 */
UCLASS()
class UNREALSHARP_API AUnrealSharpDemoActor : public AActor
{
	GENERATED_BODY()

public:
	AUnrealSharpDemoActor();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(meta = (DotnetReadWrite = "true", Access = "public|member_function_name|empty")) 
	uint32 Int32Prop;

	UPROPERTY(meta = (DotnetReadWrite = "true")) 
	uint64 Int64Prop; 
	
	UPROPERTY(meta = (DotnetReadWrite = "true")) 
	float  FloatProp;

	UPROPERTY(meta = (DotnetReadWrite = "true"))
	double  DoubleProp;
	
	TObjectPtr<UUnrealSharpSubsystem> UnrealSharpSubsystem;
	LambdaSnail::UnrealSharp::ActorHandle UnrealSharpActorHandle;
	
public:

	virtual void Tick(float DeltaTime) override;
};
