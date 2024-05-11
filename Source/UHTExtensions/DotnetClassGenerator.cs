using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;
using LambdaSnail.UnrealSharp.UHT.Extensions.Settings;

namespace LambdaSnail.UnrealSharp.UHT.Extensions;

public class DotnetClassGenerator
{
	private readonly IUhtExportFactory _factory;
	private readonly UnrealSharpConfiguration Config;
	private readonly TypeMapper TypeMap;
	private static readonly string DotnetClassNameSpecifier = "DotnetClassName";
	private static readonly UhtMetaDataKey ClassNameKey = new UhtMetaDataKey(DotnetClassNameSpecifier);

	public DotnetClassGenerator(IUhtExportFactory factory, UnrealSharpConfiguration config, TypeMapper typeMap)
	{
		_factory = factory;
		Config = config;
		TypeMap = typeMap;
	}
	
	public void EmitClass(UhtClass @class, List<PropertyDescriptor> properties, StringBuilder builder)
	{
		// var config = await ConfigurationManager.GetConfigurationIfExists(_factory.Session.ProjectDirectory!);
		// if (config is null)
		// {
		// 	_factory.Session.LogError("Unable to find configuration file in {ProjectDirectory}", _factory.Session.ProjectDirectory);
		// 	return;
		// }
		
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
		EmitStaticClassForBindings(properties, builder, bindingsClassName, Config);
		CommitGeneratedCode(@class, builder, bindingsClassName, Config);
		builder.Clear();
		
		EmitClassWithProperties(properties, builder, className, bindingsClassName, Config);
		CommitGeneratedCode(@class, builder, className, Config);
	}

	private void EmitClassWithProperties(List<PropertyDescriptor> properties, StringBuilder builder, string className, string bindingsClassName, UnrealSharpConfiguration config)
	{
		EmitFileScopeNamespace(builder, className, config);
		
		builder.Append("public partial class ");
		builder.Append(className);
		builder.Append('{');
		builder.AppendLine();

		foreach (var property in properties)
		{
			_factory.Session.LogInfo("Generating property {Property}", property.Property.GetDisplayNameText());
			if (false == TypeMap.TryGetTypeDescriptor(property.Property, out DotnetTypeDescriptor? typeDescriptor))
			{
				continue;
			}
			
			builder.Append("public ");
			property.EmitPropertyTypeString(builder);
			builder.Append(" ");
			builder.AppendLine(property.Property.GetDisplayNameText());
			
			// Property Getter
			builder.AppendLine("{ get {");
			builder.Append(bindingsClassName);
			builder.Append('.');
			builder.Append(property.GetNameOfGetter());
			builder.Append("(ActorPtr, out ");
			
			builder.Append(typeDescriptor.DotnetTypeName);
			//property.EmitPropertyTypeString(builder);  
			
			builder.AppendLine(" val); return val;");
			
			// Property Setter
			builder.AppendLine("} set {");
			builder.Append(bindingsClassName);
			builder.Append('.');
			builder.Append(property.GetNameOfSetter());
			builder.Append("(ActorPtr, ref value);");
			
			builder.AppendLine("}");
			builder.AppendLine("}");
			builder.AppendLine();
		}
		
	// public Vector Translation
	// {
	// 	get
	// 	{
	// 		ActorManager.GetTranslation(ActorPtr, out Vector vector);
	// 		return vector;
	// 	}
	// 	set
	// 	{
	// 		ActorManager.SetTranslation(ActorPtr, ref value);
	// 	}
	// }
		
		builder.Append('}'); // End of class
	}

	private void CommitGeneratedCode(UhtClass @class, StringBuilder builder, string className, UnrealSharpConfiguration config)
	{
		//string fullPath = Path.Combine(@class.Package.Module.OutputDirectory, bindingsClassName + ".dotnetintegration.cs");
		string fullPath = Path.Combine(config.DotnetProjectDirectory, className + ".g.cs");
		_factory.CommitOutput(fullPath, builder.ToString());
	}

	private static void EmitStaticClassForBindings(List<PropertyDescriptor> properties, StringBuilder builder, string className, UnrealSharpConfiguration config)
	{
		EmitFileScopeNamespace(builder, className, config);

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

	private static void EmitFileScopeNamespace(StringBuilder builder, string className, UnrealSharpConfiguration config)
	{
		builder.Append("namespace ");
		builder.Append(config.NamespaceSettings.DefaultNamespace);
		if (config.NamespaceSettings.NamespacePerClass)
		{
			builder.Append(".");
			builder.Append(className);
		}
		
		builder.AppendLine(";"); // Close namespace declaration
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