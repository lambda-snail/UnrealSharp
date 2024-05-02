#pragma once

// TODO: Can we find a way to use the FTransform or auto generate this kind of structure?
// TODO: Struct with single member compiles but crashes at runtime
struct Transform
{
	FVector3d _R;
	FVector3d _T;
	FVector3d _S;
};

struct Vector3d
{
	double _X;
	double _Y;
	double _Z;
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

// extern "C" __declspec(dllexport) inline FVector GetTranslation(AActor const* Actor)
// {
// 	return Actor->GetActorLocation();
// }

extern "C" __declspec(dllexport) inline Vector3d GetTranslation(AActor const* Actor)
{
	//FVector* Vec = static_cast<FVector*>(Vector);
	//*Vec = Actor->GetActorLocation();
	FVector Location = Actor->GetActorLocation();
	return { ._X = Location.X, ._Y = Location.Y, ._Z = Location.Z };
}

// extern "C" __declspec(dllexport) inline FVector GetRotation(AActor const* Actor)
// {
// 	return Actor->GetActorRotation().Euler();
// }
//
// extern "C" __declspec(dllexport) inline FVector GetScale(AActor const* Actor)
// {
// 	return Actor->GetActorScale();
// }
//
extern "C" __declspec(dllexport) inline void SetTransform(AActor* Actor, Transform Transform)
{
	Actor->SetActorLocation(Transform._T);
	Actor->SetActorScale3D(Transform._S);
	Actor->SetActorRotation(FRotator::MakeFromEuler(Transform._R));
}
//
// extern "C" __declspec(dllexport) inline void SetLocation(AActor* Actor, FVector Location)
// {
// 	Actor->SetActorLocation(Location);
// }
//
// extern "C" __declspec(dllexport) inline void SetRotation(AActor* Actor, FVector Rotation)
// {
// 	Actor->SetActorRotation(FRotator::MakeFromEuler(Rotation));
// }
//
// extern "C" __declspec(dllexport) inline void SetScale(AActor* Actor, FVector Scale)
// {
// 	Actor->SetActorScale3D(Scale);
// }
