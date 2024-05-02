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

	// TODO: Why is this handled as public without the DotnetAccess?
	UPROPERTY(meta = (DotnetReadWrite = "true")) 
	uint64 Int64Prop; 

	// TODO: Handle constness as parameter as well?
	// TODO: Look over the metadata api for property binding generation - use a DSL?
	double GetDoubleProp() const { return {}; };
	void SetDoubleProp(double Prop) {};
	
protected:

	virtual void BeginPlay() override;

	UPROPERTY(meta = (DotnetReadWrite = "true", DotnetAccess = "Reflection")) 
	uint32 Int32Prop;
	
	UPROPERTY(meta = (DotnetReadWrite = "true", DotnetAccess = "Reflection")) 
	float  FloatProp;

	UPROPERTY(meta = (DotnetReadWrite = "true", DotnetAccess = "DoubleProp"))
	double  DoubleProp;
	
	TObjectPtr<UUnrealSharpSubsystem> UnrealSharpSubsystem;
	LambdaSnail::UnrealSharp::ActorHandle UnrealSharpActorHandle;
	
public:

	virtual void Tick(float DeltaTime) override;
};
