using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;
using LambdaSnail.UnrealSharp.UHT.Extensions.Settings;
using UnrealBuildBase;
using VYaml.Serialization;

namespace LambdaSnail.UnrealSharp.UHT.Extensions;

public class DotnetClassGenerator
{
	private readonly IUhtExportFactory _factory;
	private static readonly string DotnetClassNameSpecifier = "DotnetClassName";
	private static readonly UhtMetaDataKey ClassNameKey = new UhtMetaDataKey(DotnetClassNameSpecifier);

	private UnrealSharpConfiguration CodeGenerationSettings { get; set; } = default!;
	
	public DotnetClassGenerator(IUhtExportFactory factory)
	{
		_factory = factory;
	}
	
	public async Task EmitClass(UhtClass @class, List<PropertyDescriptor> properties, StringBuilder builder)
	{
		var config = await ConfigurationManager.GetConfigurationIfExists(_factory.Session.ProjectDirectory!);
		if (config is null)
		{
			_factory.Session.LogError("Unable to find configuration file in {ProjectDirectory}", _factory.Session.ProjectDirectory);
			return;
		}

		string className = @class.GetDisplayNameText();
		if (@class.MetaData.Dictionary?.TryGetValue(ClassNameKey, out string? name) is true)
		{
			className = name;
		}
		
		// Generate C# bindings:
		// - Generate LibraryImport declarations
		//	- Do we put these in own static class or in the same class?
		// - Check if we need to generate a type
		//	- Do we need to maintain a type mapping somewhere?
		// - Do we accept strings for now?
		
		string bindingsClassName = className + "_Bindings"; 
		EmitStaticClassForBindings(properties, builder, bindingsClassName);
		CommitGeneratedCode(@class, builder, bindingsClassName);
	}

	private void CommitGeneratedCode(UhtClass @class, StringBuilder builder, string bindingsClassName)
	{
		string fullPath = Path.Combine(@class.Package.Module.OutputDirectory, bindingsClassName + ".dotnetintegration.cs");
		_factory.CommitOutput(fullPath, builder.ToString());
		_factory.Session.LogInfo($"Exported file {fullPath}");
	}

	private static void EmitStaticClassForBindings(List<PropertyDescriptor> properties, StringBuilder builder, string className)
	{
		builder.Append("public static partial class ");
		builder.Append(className);
		builder.Append('{');
	
		foreach (PropertyDescriptor descriptor in properties)
		{
			EmitLibraryImportDeclarationForPropertyAccess(descriptor.GetNameOfGetter(), builder, descriptor);
			if (descriptor.AccessMode == AccessMode.ReadWrite)
			{
				EmitLibraryImportDeclarationForPropertyAccess(descriptor.GetNameOfSetter(), builder, descriptor);
			}
		}
		
		builder.Append('}'); // End static class
	}

	/// <summary>
	/// Emits a LibraryImport declaration for a function to access (read or write) a property.
	/// </summary>
	private static void EmitLibraryImportDeclarationForPropertyAccess(string accessFunctionName, StringBuilder builder, PropertyDescriptor descriptor)
	{
		// [LibraryImport(Constants.DebugDllName, EntryPoint = "SetPropertyName")]
		builder.Append("[LibraryImport(Constants.DebugDllName, EntryPoint = \"");
		builder.Append(accessFunctionName);
		builder.AppendLine("\")]");
			
		// internal static unsafe partial void SetPropertyName(IntPtr actor, ref Vector vector);
		builder.Append("internal static unsafe partial void ");
		builder.Append(accessFunctionName);
		builder.Append("(IntPtr instance, ref ");
		descriptor.EmitPropertyTypeString(builder);
		builder.AppendLine(" value);");
	}
}