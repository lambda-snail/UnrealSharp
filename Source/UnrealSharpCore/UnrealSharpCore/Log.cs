using System.Runtime.InteropServices;

namespace LambdaSnail.UnrealSharp;

public static unsafe class UELog
{
    private static delegate*<IntPtr, void> UeLog;
    
    public static void Log(string message)
    {
        UeLog(RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? Marshal.StringToCoTaskMemUni(message)
            : Marshal.StringToCoTaskMemUTF8(message));
    }

    [UnmanagedCallersOnly]
    public static void BindLogger(delegate*<IntPtr, void> logger)
    {
        UeLog = logger;
    }
}