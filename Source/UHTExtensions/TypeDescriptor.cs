using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using EpicGames.UHT.Types;
using LambdaSnail.UnrealSharp.UHT.Extensions.Settings;
using VYaml.Annotations;

namespace LambdaSnail.UnrealSharp.UHT.Extensions;

[YamlObject(NamingConvention.KebabCase)]
public partial class DotnetTypeDescriptor
{
	/// <summary>
	/// The name of the type, without namespace qualification.
	/// </summary>
	public string DotnetTypeName { get; set; } = default!;
	
	/// <summary>
	/// The namespace that contains the definition of a type in dotnet.
	/// </summary>
	public string DotnetNamespace { get; set; } = string.Empty;
	
	public bool IsPrimitive { get; set; }

	/// <summary>
	/// Returns the type name qualified by the namespace.
	/// </summary>
	public string GetFqnDotnet()
	{
		return IsPrimitive ? DotnetTypeName : DotnetNamespace + "." + DotnetTypeName;
	}
}

public class TypeMapper
{
	public Dictionary<string, DotnetTypeDescriptor> Map;
	
	public TypeMapper(UnrealSharpConfiguration config)
	{
		DotnetTypeDescriptor intType    = new() { DotnetTypeName = "int", IsPrimitive = true};
		DotnetTypeDescriptor uintType   = new() { DotnetTypeName = "uint", IsPrimitive = true};
		DotnetTypeDescriptor int64Type  = new() { DotnetTypeName = "long", IsPrimitive = true};
		DotnetTypeDescriptor uint64Type  = new() { DotnetTypeName = "ulong", IsPrimitive = true};
		DotnetTypeDescriptor floatType  = new() { DotnetTypeName = "float", IsPrimitive = true};
		DotnetTypeDescriptor doubleType = new() { DotnetTypeName = "double", IsPrimitive = true};
		DotnetTypeDescriptor boolType   = new() { DotnetTypeName = "bool", IsPrimitive = true};
		
		DotnetTypeDescriptor vectorType   = new()
		{
			DotnetTypeName = "Vector", 
			DotnetNamespace = "LambdaSnail.UnrealSharp", 
			IsPrimitive = false
		};
		
		Map = new()
		{
			{ "int32", intType }, { "int", intType }, // TODO: int may not be 32 bits on all machines in C++ 
			{ "uint32", uintType }, { "uint", uintType },
			{ "int64", int64Type },
			{ "uint64", uint64Type },
			{ "float", floatType }, 
			{ "double", doubleType },
			{ "bool", boolType },
			
			{ "FVector", vectorType } // TODO: FVector has many variations - needs more work, but good enough for now
		};
		
		foreach (var (type, descriptor) in config.TypeMappings.MapOverrides)
		{
			Map[type] = descriptor;
		}
	}

	public bool TryGetTypeDescriptor(UhtProperty uProperty, [MaybeNullWhen(false)] out DotnetTypeDescriptor typeDescriptor)
	{
		// Super ugly hack to get the CppTypeText
		// This property doesn't have a getter since it was never intended to be used in this way,
		// instead in exporters, 'getting' this property is the same as appending it to the output string
		Type type = typeof(UhtProperty);
		PropertyInfo? cppTypeText = type.GetProperty("CppTypeText", BindingFlags.NonPublic | BindingFlags.Instance);
		ArgumentNullException.ThrowIfNull(cppTypeText);
		
		string? cppType = cppTypeText.GetValue(uProperty) as string;
		if(cppType is null)
		{
			typeDescriptor = null;
			return false;
		}

		typeDescriptor = Map[cppType];
		return true;
	}
}