﻿using System.Runtime.InteropServices;

namespace LambdaSnail.UnrealSharp;

using unsafe get_transformdelegate = delegate*<Transform>;
using unsafe set_transformdelegate = delegate*<Transform, void>;
using ActorHandle = int;

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
    public Vector Location;
}

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
    
    public abstract void Tick(float deltaTime);

    protected unsafe Transform GetTransform()
    {
        return get_transform();
    }
    
    protected unsafe void SetTransform(Transform transform)
    {
        set_transform(transform);
    }
}