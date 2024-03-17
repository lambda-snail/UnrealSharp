using System.Runtime.InteropServices;

namespace LambdaSnail.UnrealSharp;

using unsafe get_transformdelegate = delegate*<Transform>;
using ActorHandle = int;

[StructLayout(LayoutKind.Sequential)]
public struct Vector
{
    public float X;
    public float Y;
    public float Z;
}

[StructLayout(LayoutKind.Sequential)]
public struct Transform
{
    public Vector Location;
}

public abstract class Actor
{
    private unsafe get_transformdelegate get_transform;
    
    public unsafe void BindDelegates(get_transformdelegate get_transform_in)
    {
        this.get_transform = get_transform_in;
    }

    public Actor() {}
    
    public ActorHandle ActorHandle { get; set; }
    
    public abstract void Tick(float deltaTime);

    protected unsafe Transform GetTransform()
    {
        //return Marshal.PtrToStructure<Transform>(get_transform());
        return get_transform();
    }
}

public class SomeActor : Actor
{
    public override void Tick(float deltaTime)
    {
        Transform t = GetTransform();
        UELog.Log($"Actor with handle {ActorHandle}: Transform=({t.Location.X},{t.Location.Y},{t.Location.Z})");
    }
}