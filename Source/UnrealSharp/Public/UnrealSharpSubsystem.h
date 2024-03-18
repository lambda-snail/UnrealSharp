// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Host.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UnrealSharpSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UNREALSHARP_API UUnrealSharpSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	LambdaSnail::UnrealSharp::ActorHandle RegisterActorForTick(AActor* Actor);
	void TickActor(LambdaSnail::UnrealSharp::ActorHandle Handle, float DeltaTime);
	
private:
	LambdaSnail::UnrealSharp::ManagedActorFunctions ActorFunctions;
	
	TMap<int, TStrongObjectPtr<AActor>> RegisteredActors;
};
