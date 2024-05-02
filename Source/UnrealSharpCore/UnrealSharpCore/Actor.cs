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
    public Vector Rotation;
    public Vector Translation;
    public Vector Scale;
}

public abstract partial class Actor
{
    public Actor() {}
    
    public ActorHandle ActorHandle { get; set; }

    public unsafe IntPtr ActorPtr { get; set; }

    public abstract void Tick(float deltaTime);

    
    public Vector Translation
    {
        get
        {
            ActorManager.GetTranslation(ActorPtr, out Vector vector);
            return vector;
        }
        set
        {
            ActorManager.SetTranslation(ActorPtr, ref value);
        }
    }
    
    public Vector Rotation
    {
        get
        {
            ActorManager.GetRotation(ActorPtr, out Vector vector);
            return vector;
        }
        set
        {
            ActorManager.SetRotation(ActorPtr, ref value);
        }
    }
    
    public Vector Scale
    {
        get
        {
            ActorManager.GetScale(ActorPtr, out Vector vector);
            return vector;
        }
        set
        {
            ActorManager.SetScale(ActorPtr, ref value);
        }
    }
}
