// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#pragma once

#if defined(_WIN32)
    #define CORECLR_DELEGATE_CALLTYPE __stdcall
#else
    #define CORECLR_DELEGATE_CALLTYPE
#endif

#define UNMANAGEDCALLERSONLY_METHOD ((const TCHAR*)-1)

// Signature of delegate returned by coreclr_delegate_type::load_assembly_and_get_function_pointer
typedef int (CORECLR_DELEGATE_CALLTYPE *LoadAssemblyAndGetFunctionPointer_Fn)(
    const TCHAR*   AssemblyPath      /* Fully qualified path to assembly */,
    const TCHAR*   TypeName          /* Assembly qualified type name */,
    const TCHAR*   MethodName        /* Public static method name compatible with delegateType */,
    const TCHAR*   DelegateTypeName /* Assembly qualified delegate type name or null
                                        or UNMANAGEDCALLERSONLY_METHOD if the method is marked with
                                        the UnmanagedCallersOnlyAttribute. */,
    void*           Reserved           /* Extensibility parameter (currently unused and must be 0) */,
    /*out*/ void**  Delegate          /* Pointer where to store the function pointer result */);

// Signature of delegate returned by load_assembly_and_get_function_pointer_fn when delegate_type_name == null (default)
typedef int (CORECLR_DELEGATE_CALLTYPE *ComponentEntryPoint_Fn)(void *Arg, int32 ArgSizeInBytes);

typedef int (CORECLR_DELEGATE_CALLTYPE *GetFunctionPointer_Fn)(
    const TCHAR*    TypeName          /* Assembly qualified type name */,
    const TCHAR*    MethodName        /* Public static method name compatible with delegateType */,
    const TCHAR*    DelegateTypeName /* Assembly qualified delegate type name or null,
                                        or UNMANAGEDCALLERSONLY_METHOD if the method is marked with
                                        the UnmanagedCallersOnlyAttribute. */,
    void*           LoadContext       /* Extensibility parameter (currently unused and must be 0) */,
    void*           Reserved           /* Extensibility parameter (currently unused and must be 0) */,
    /*out*/ void**  Delegate          /* Pointer where to store the function pointer result */);
