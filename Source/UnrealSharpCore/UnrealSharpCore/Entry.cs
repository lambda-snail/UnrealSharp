using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text.Unicode;

[module: System.Runtime.InteropServices.DefaultCharSet( CharSet.Unicode )]
[assembly: System.Runtime.CompilerServices.DisableRuntimeMarshallingAttribute]

namespace LambdaSnail.UnrealSharp;

using ActorHandle = int;
using unsafe get_transformdelegate = delegate*<int, Transform>;
using unsafe set_transformdelegate = delegate*<int, Transform, void>;

public static class Constants
{
    public const string DebugDllName = "UnrealEditor-UnrealSharp-Win64-DebugGame.dll";
}

public static partial class ActorManager
{
    static ActorManager()
    {
        NativeLibrary.SetDllImportResolver(Assembly.GetAssembly(typeof(ActorManager))!, (name, _, paths) =>
        {
            if (name == "__Internal")
            {
                return NativeLibrary.GetMainProgramHandle(); // After dotnet 7, https://github.com/dotnet/runtime/issues/56331
            }
            return IntPtr.Zero;
        });
    }
    
    // TODO: Find a better way to handle dll names - __Internal doesn't seem to work here
    //[LibraryImport("__Internal", EntryPoint = "TestActorFunctions")]
    [LibraryImport(Constants.DebugDllName, EntryPoint = "SetTransform")]
    internal static partial void SetTransform(IntPtr actor, Transform transform);
    
    [LibraryImport(Constants.DebugDllName, EntryPoint = "GetTransform")]
    internal static partial Transform GetTransform(IntPtr actor);
    
    [LibraryImport(Constants.DebugDllName, EntryPoint = "GetTranslation")]
    internal static unsafe partial void GetTranslation(IntPtr actor, out Vector vector);
    
    [LibraryImport(Constants.DebugDllName, EntryPoint = "GetRotation")]
    internal static unsafe partial void GetRotation(IntPtr actor, out Vector vector);
    
    [LibraryImport(Constants.DebugDllName, EntryPoint = "GetScale")]
    internal static unsafe partial void GetScale(IntPtr actor, out Vector vector);
    
    
    [LibraryImport(Constants.DebugDllName, EntryPoint = "SetTranslation")]
    internal static unsafe partial void SetTranslation(IntPtr actor, ref Vector vector);
    
    [LibraryImport(Constants.DebugDllName, EntryPoint = "SetRotation")]
    internal static unsafe partial void SetRotation(IntPtr actor, ref Vector vector);
    
    [LibraryImport(Constants.DebugDllName, EntryPoint = "SetScale")]
    internal static unsafe partial void SetScale(IntPtr actor, ref Vector vector);
    
    
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
    public static ActorHandle RegisterActor(IntPtr assembly, IntPtr type, IntPtr nativeActor)
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
        actor.ActorPtr = nativeActor;
        actor.ActorHandle = NextActorHandle++;
        Actors.Add(actor.ActorHandle, actor);
        
        UELog.Log($"Registered Actor with handle {actor.ActorHandle}");
        
        return actor.ActorHandle;
    }
}
