namespace LambdaSnail.UnrealSharp;

using unsafe get_transformdelegate = delegate*<Transform> ;
using ActorHandle = int;

public struct Vector
{
    public double X;
    public double Y;
    public double Z;
}

public struct Transform
{
    public Vector Location;
}

public abstract class Actor
{
    private unsafe get_transformdelegate get_transform;
    
    public unsafe void InitDelegates(get_transformdelegate get_transform_in)
    {
        this.get_transform = get_transform_in;
    }

    public Actor() {}
    
    public ActorHandle ActorHandle { get; set; }
    
    public abstract void Tick(float deltaTime);

    public unsafe Transform GetTransform()
    {
        return get_transform();
    }
}

public class SomeActor : Actor
{
    public override void Tick(float deltaTime)
    {
        Transform t = GetTransform();
    }
}