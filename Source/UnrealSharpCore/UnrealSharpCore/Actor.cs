using System.Runtime.InteropServices;

namespace LambdaSnail.UnrealSharp;

using ActorHandle = int;
using unsafe get_transformdelegate = delegate*<int, Transform>;
using unsafe set_transformdelegate = delegate*<int, Transform, void>;

[StructLayout(LayoutKind.Sequential)]
public struct Vector
{
    public double X;
    public double Y;
    public double Z;
}

[StructLayout(LayoutKind.Sequential)]
public struct Transform
{
    //public Vector Location;
    public Vector Rotation;
    public Vector Translation;
    public Vector Scale;
}

// [StructLayout(LayoutKind.Sequential)]
// public struct TransformWrapper
// {
//     public Transform Transform;
// }

public abstract class Actor
{
    private unsafe get_transformdelegate get_transform;
    private unsafe set_transformdelegate set_transform;
    
    public unsafe void BindDelegates(get_transformdelegate get_transform_in, set_transformdelegate set_transform_in)
    {
        this.get_transform = get_transform_in;
        this.set_transform = set_transform_in;
    }

    public Actor() {}
    
    public ActorHandle ActorHandle { get; set; }

    public unsafe IntPtr ActorPtr { get; set; }

    public abstract void Tick(float deltaTime);

    // protected unsafe Transform GetTransform()
    // {
    //     return get_transform(ActorHandle);
    // }
    
    // protected unsafe void SetTransform(Transform transform)
    // {
    //     set_transform(ActorHandle, transform);
    // }
}
