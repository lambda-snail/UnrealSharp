// Fill out your copyright notice in the Description page of Project Settings.

#include "Demo/UnrealSharpDemoActor.h"

#include "AUnrealSharpDemoActor.dotnetintegration.g.h"

#include "UnrealSharpSubsystem.h"


// Sets default values
AUnrealSharpDemoActor::AUnrealSharpDemoActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

/**
 * In begin play we first obtain a pointer to the UUnrealSharpSubsystem, then we use that to register our actor by calling
 * RegisterActorForTick. This will create an instance of an actor class on the dotnet side, which will be able to control
 * the c++ actor on each tick.
 */
void AUnrealSharpDemoActor::BeginPlay()
{
	UnrealSharpSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UUnrealSharpSubsystem>();
	UnrealSharpActorHandle = UnrealSharpSubsystem->RegisterActorForTick(this);
	Super::BeginPlay();

	
}

/**
 * After registering an actor in BeginPlay, we can request a tick to be performed by the hosted dotnet runtime. We can do this
 * each tick or less frequently if we like - but remember to pass in the correct delta time so that the calculations can be
 * done correctly.
 */
void AUnrealSharpDemoActor::Tick(float DeltaTime)
{
	UnrealSharpSubsystem->TickActor(UnrealSharpActorHandle, DeltaTime);

	FProperty* Property = this->GetClass()->FindPropertyByName("Int32Prop");
	if(FNumericProperty* NumericProperty = Cast<FNumericProperty>(Property))
	{
		int x = NumericProperty->GetSignedIntPropertyValue(this);
	}

	Int32Prop = 32;
	
	int32 P;
	Property->GetValue_InContainer(this, &P);
	
	FProperty* Property2 = this->GetClass()->FindPropertyByName("FloatProp");
	if(FNumericProperty* NumericProperty = Cast<FNumericProperty>(Property2))
	{
		float x = NumericProperty->GetSignedIntPropertyValue(this);
	}

	FloatProp = 99.87;
	
	float F;
	Property2->GetValue_InContainer(this, &F);
}
