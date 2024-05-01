#pragma once

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
// TODO: Add functions for Translation, Rotation and Scale individually, so that we can set them without
//	copying stuff that haven't changed
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