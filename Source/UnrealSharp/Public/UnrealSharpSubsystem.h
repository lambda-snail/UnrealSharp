// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Host.h"
#include "LambdaHelpers.h"
#include "Logging/StructuredLog.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/PropertyAccessUtil.h"
#include "UObject/UnrealTypePrivate.h"
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

	template<typename TProperty>
	void RegisterNumericProperty(LambdaSnail::UnrealSharp::ActorHandle Handle, FNumericProperty* Property);

	template<typename TProperty>
	void RegisterNumericProperty(LambdaSnail::UnrealSharp::ActorHandle Handle, FName PropertyName);
	
private:
	LambdaSnail::UnrealSharp::ManagedActorFunctions ActorFunctions;
	
	TMap<int, TStrongObjectPtr<AActor>> RegisteredActors;
};

// TODO: Can we find a way to use the FTransform or auto generate this kind of structure?
// TODO: Struct with single member compiles but crashes at runtime
struct Transform
{
	//FTransform T;
	FVector3d _R;
	FVector3d _T;
	FVector3d _S;
};

// TODO: Remove lambda based api and use these instead
// TODO: Return something that is compatible with C
// TODO: What happens with the ptr when an actor gets destroyed?
extern "C" __declspec(dllexport) inline Transform GetTransform(AActor const* Actor)
{
	FTransform T = Actor->GetTransform();
	return { ._R = T.GetRotation().Euler(), ._T = T.GetTranslation(), ._S = T.GetScale3D() };
}

extern "C" __declspec(dllexport) inline void SetTransform(AActor* Actor, Transform Transform)
{
	Actor->SetActorLocation(Transform._T);
	Actor->SetActorScale3D(Transform._S);
	Actor->SetActorRotation(FRotator::MakeFromEuler(Transform._R));
}


// TOREAD: https://forums.unrealengine.com/t/correct-way-to-get-the-value-of-a-property/551897/3
template <typename TProperty>
void UUnrealSharpSubsystem::RegisterNumericProperty(LambdaSnail::UnrealSharp::ActorHandle Handle, FNumericProperty* Property)
{
	static_assert(std::is_arithmetic_v<TProperty>);
	using namespace LambdaSnail::UnrealSharp;
	
	// ActorFunctions.BindDelegates(Handle, Lambda::ToFunctionPointer([this, Property](ActorHandle ActorHandle)
	// 	{
	// 		// if constexpr (std::is_integral_v<TProperty>)
	// 		// {
	// 		// 	Property->GetSignedIntPropertyValue_InContainer();
	// 		// 	
	// 		// 	Property->SetIntPropertyValue(this->RegisteredActors[ActorHandle], NewValue);
	// 		// }
	//
	// 		AActor* RawPtr = &(*this->RegisteredActors[ActorHandle]);
	// 		TProperty* Value = Property->ContainerPtrToValuePtr<TProperty>(RawPtr);
	// 		return *Value;
	// 	}),
	// 	Lambda::ToFunctionPointer([this, Property](ActorHandle ActorHandle, TProperty NewValue)
	// 	{
	// 		// TProperty* Value = Property->ContainerPtrToValuePtr<TProperty>(this->RegisteredActors[ActorHandle]->GetActorLocation());
	// 		// Value = NewValue;
	//
	// 		AActor* RawPtr = &(*this->RegisteredActors[ActorHandle]);
	// 		if constexpr (std::is_integral_v<TProperty>)
	// 		{
	// 			Property->SetIntPropertyValue(RawPtr, static_cast<uint64>(NewValue)); // TODO: bind_delegates_fn has hard coded SimpleTransform
	// 		}
	// 		else if constexpr (std::is_floating_point_v<TProperty>)
	// 		{
	// 			Property->SetFloatingPointPropertyValue(RawPtr, static_cast<uint64>(NewValue));
	// 		}
	// 	}
	// ));
}

template <typename TProperty>
void UUnrealSharpSubsystem::RegisterNumericProperty(LambdaSnail::UnrealSharp::ActorHandle Handle, FName PropertyName)
{
	static_assert(std::is_arithmetic_v<TProperty>);
	
	FProperty* Property = this->RegisteredActors[Handle]->GetClass()->FindPropertyByName(PropertyName);
	if(FNumericProperty* NumericProperty = Cast<FNumericProperty>(Property))
	{
		RegisterNumericProperty<TProperty>(Handle, NumericProperty);
	}
}
