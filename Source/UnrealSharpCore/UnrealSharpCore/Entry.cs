using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.Unicode;

[module: System.Runtime.InteropServices.DefaultCharSet( CharSet.Unicode )]

namespace LambdaSnail.UnrealSharp;

using ActorHandle = int;

public class ActorComparer : IEqualityComparer<Actor>
{
    public bool Equals(Actor x, Actor y)
    {
        return x.ActorHandle == y.ActorHandle;
    }

    public int GetHashCode(Actor obj)
    {
        return obj.ActorHandle;
    }
}

public static class ActorManager
{
    private static HashSet<Actor> Actors { get; set; } = default!;
    private static ActorHandle NextActorHandle = 0;
    
    //[UnmanagedCallersOnly]
    public static int InitActorManager(IntPtr arg, int argLength)
    {
        Console.WriteLine($"Init has been called!");
        Actors = new(new ActorComparer());

        return 0;
    }

    [UnmanagedCallersOnly]
    public static void TickActors(float deltaTime)
    {
        foreach (Actor a in Actors)
        {
            a.Tick(deltaTime);
        }
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
        Actors.Add(actor);
        
        return actor.ActorHandle;
    }
}

// namespace DotNetLib
// {
//     public static class Lib
//     {
//         private static int s_CallCount = 1;
//
//         [StructLayout(LayoutKind.Sequential)]
//         public struct LibArgs
//         {
//             public IntPtr Message;
//             public int Number;
//         }
//
//         public static int Hello(IntPtr arg, int argLength)
//         {
//             if (argLength < System.Runtime.InteropServices.Marshal.SizeOf(typeof(LibArgs)))
//             {
//                 return 1;
//             }
//
//             LibArgs libArgs = Marshal.PtrToStructure<LibArgs>(arg);
//             Console.WriteLine($"Hello, world! from {nameof(Lib)} [count: {s_CallCount++}]");
//             PrintLibArgs(libArgs);
//             return 0;
//         }
//
//         [UnmanagedCallersOnly]
//         public static unsafe void TestFnPtr(delegate*<void> fn_from_cpp)
//         {
//             Console.WriteLine($"[C#] Preparing to call c++ function");
//             fn_from_cpp();
//         }
//         
//         [UnmanagedCallersOnly]
//         public static unsafe void TestFnPtrWithArgs(delegate* unmanaged<int, double> fn_from_cpp)
//         {
//             Console.WriteLine($"[C#] Entering {nameof(TestFnPtrWithArgs)}");
//             double ret = fn_from_cpp(20);
//             Console.WriteLine($"[C#] C++ function returned {ret}!");
//         }
//
//         [UnmanagedCallersOnly]
//         public static unsafe void TestStringInputOutput(delegate* unmanaged<IntPtr, IntPtr> str_fn)
//         {
//             Console.WriteLine($"[C#] Entering {nameof(TestStringInputOutput)}");
//
//             string str_from_cs = "String from c#";
//             IntPtr cpp_str = str_fn(RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
//                                         ? Marshal.StringToCoTaskMemUni(str_from_cs)
//                                         : Marshal.StringToCoTaskMemUTF8(str_from_cs));
//             
//             
//             
//             string cs_str = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
//                 ? Marshal.PtrToStringUni(cpp_str)!
//                 : Marshal.PtrToStringUTF8(cpp_str)!;
//             
//             Console.WriteLine($"[C#] String from c++: {cs_str}");
//         }
//         
//         public delegate void CustomEntryPointDelegate(LibArgs libArgs);
//         public static void CustomEntryPoint(LibArgs libArgs)
//         {
//             Console.WriteLine($"Hello, world! from {nameof(CustomEntryPoint)} in {nameof(Lib)}");
//             PrintLibArgs(libArgs);
//         }
//
//         [UnmanagedCallersOnly]
//         public static void CustomEntryPointUnmanagedCallersOnly(LibArgs libArgs)
//         {
//             Console.WriteLine($"Hello, world! from {nameof(CustomEntryPointUnmanagedCallersOnly)} in {nameof(Lib)}");
//             PrintLibArgs(libArgs);
//         }
//
//         private static void PrintLibArgs(LibArgs libArgs)
//         {
//             string message = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
//                 ? Marshal.PtrToStringUni(libArgs.Message)!
//                 : Marshal.PtrToStringUTF8(libArgs.Message)!;
//
//             Console.WriteLine($"-- message: {message}");
//             Console.WriteLine($"-- number: {libArgs.Number}");
//         }
//     }
// }
