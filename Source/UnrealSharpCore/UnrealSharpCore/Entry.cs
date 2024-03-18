using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.Unicode;

[module: System.Runtime.InteropServices.DefaultCharSet( CharSet.Unicode )]

namespace LambdaSnail.UnrealSharp;

using ActorHandle = int;
using unsafe get_transformdelegate = delegate*<int, Transform>;
using unsafe set_transformdelegate = delegate*<int, Transform, void>;

public static class ActorManager
{
    private static Dictionary<ActorHandle, Actor> Actors { get; set; } = default!;
    private static ActorHandle NextActorHandle = 0;
    
    //[UnmanagedCallersOnly]
    public static int InitActorManager(IntPtr arg, int argLength)
    {
        Console.WriteLine($"[dotnet] Initializing ActorManager");
        Actors = new();

        return 0;
    }

    [UnmanagedCallersOnly]
    public static void TickActors(float deltaTime)
    {
        foreach (Actor a in Actors.Values)
        {
            a.Tick(deltaTime);
        }
    }
    
    [UnmanagedCallersOnly]
    public static void TickSingleActor(ActorHandle handle, float deltaTime)
    {
        Actors[handle].Tick(deltaTime);
    }
    
    [UnmanagedCallersOnly]
    public static void TickActor(ActorHandle handle, float deltaTime)
    {
        Actors[handle].Tick(deltaTime);
    }

    [UnmanagedCallersOnly]
    public static unsafe void BindDelegates(ActorHandle handle, get_transformdelegate get_transform, set_transformdelegate set_transform)
    {
        Actors[handle].BindDelegates(get_transform, set_transform);
    }
    
    [UnmanagedCallersOnly]
    public static ActorHandle RegisterActor(IntPtr assembly, IntPtr type)
    {
        string typeName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? Marshal.PtrToStringUni(type)!
            : Marshal.PtrToStringUTF8(type)!;
        
        string assemblyName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? Marshal.PtrToStringUni(assembly)!
            : Marshal.PtrToStringUTF8(assembly)!;

        var handle = Activator.CreateInstance(assemblyName, typeName);
        var obj = handle?.Unwrap();
        if (obj is null || !obj.GetType().IsSubclassOf(typeof(Actor)))
        {
            throw new InvalidOperationException(
                $"Error: Unable to create Actor of type [{typeName}, {assemblyName}]");
        }
        
        Actor actor = (Actor)obj;
        actor.ActorHandle = NextActorHandle++;
        Actors.Add(actor.ActorHandle, actor);
        
        UELog.Log($"Registered Actor with handle {actor.ActorHandle}");
        
        return actor.ActorHandle;
    }
}
