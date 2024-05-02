#pragma once

// TODO: Can we find a way to use the FTransform or auto generate this kind of structure?
// TODO: Struct with single member compiles but crashes at runtime
struct Transform
{
	FVector3d _R;
	FVector3d _T;
	FVector3d _S;
};

// TODO: Remove lambda based api and use these instead
// TODO: Return something that is compatible with C
// TODO: What happens with the ptr when an actor gets destroyed?
//	copying stuff that haven't changed
extern "C" __declspec(dllexport) inline Transform GetTransform(AActor const* Actor)
{
	FTransform T = Actor->GetTransform();
	return { ._R = T.GetRotation().Euler(), ._T = T.GetTranslation(), ._S = T.GetScale3D() };
}

extern "C" __declspec(dllexport) inline void GetTranslation(AActor const* Actor, void* Vector)
{
	FVector* Vec = static_cast<FVector*>(Vector);
	*Vec = Actor->GetActorLocation();
}

extern "C" __declspec(dllexport) inline void GetRotation(AActor const* Actor, void* Rotator)
{
	FVector* Vec = static_cast<FVector*>(Rotator);
	*Vec = Actor->GetActorRotation().Euler();
}

extern "C" __declspec(dllexport) inline void GetScale(AActor const* Actor, void* Scale)
{
	FVector* Vec = static_cast<FVector*>(Scale);
	*Vec = Actor->GetActorScale();
}

extern "C" __declspec(dllexport) inline void SetTransform(AActor* Actor, Transform Transform)
{
	Actor->SetActorLocation(Transform._T);
	Actor->SetActorScale3D(Transform._S);
	Actor->SetActorRotation(FRotator::MakeFromEuler(Transform._R));
}

extern "C" __declspec(dllexport) inline void SetTranslation(AActor* Actor, void* Translation)
{
	Actor->SetActorLocation(*static_cast<FVector*>(Translation));
}

extern "C" __declspec(dllexport) inline void SetRotation(AActor* Actor, void* Rotation)
{
	Actor->SetActorRotation(FRotator::MakeFromEuler(*static_cast<FVector*>(Rotation)));
}

extern "C" __declspec(dllexport) inline void SetScale(AActor* Actor, void* Scale)
{
	Actor->SetActorScale3D(*static_cast<FVector*>(Scale));
}
