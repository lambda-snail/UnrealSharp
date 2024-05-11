using System.Collections.Generic;
using System.Linq.Expressions;
using VYaml.Annotations;

namespace LambdaSnail.UnrealSharp.UHT.Extensions.Settings;

[YamlObject(NamingConvention.KebabCase)]
public partial class NamespaceSettings
{
	/// <summary>
	/// The default namespace to which generated code will be added by default. Can be customized by
	/// setting <see cref="DefaultNamespace"/> or <see cref="ClassNamespaceOverrides"/>.
	/// </summary>
	public string DefaultNamespace { get; init; } = default!;
	/// <summary>
	/// If true, generated classes will be put in the namespace [DefaultNameSpace].[ClassName].
	/// If false they will be put under the default namespace. Overridden by the <see cref="ClassNamespaceOverrides"/>
	/// setting. 
	/// </summary>
	public bool NamespacePerClass { get; init; }
	public List<string> ClassNamespaceOverrides { get; init; } = new List<string>();
}

[YamlObject(NamingConvention.KebabCase)]
public partial class TypeMappingSettings
{
	/// <summary>
	/// Enables users to define their own type mappings for primitive and complex types. UnrealSharp will
	/// attempt to guess some type mappings; e.g., 'int32' in c++ will be mapped to 'int' in dotnet by default. 
	/// UnrealSharp also makes the guess that 'int' in c++ maps to 'int' in dotnet.
	///
	/// The type overrides are applied after the default type mappings are constructed, so if the machine has e.g.,
	/// 16-bit ints it is possible to map 'int' to a 'short' instead.
	///
	/// Custom types should be defined in C# first and then added to this list. At least for now there is no
	/// planned feature to make UnrealSharp attempt to construct complex types automatically.
	/// </summary>
	public Dictionary<string, DotnetTypeDescriptor> MapOverrides { get; init; } = new();
}

[YamlObject(NamingConvention.KebabCase)]
public partial class UnrealSharpConfiguration
{
	public NamespaceSettings NamespaceSettings { get; init; } = default!;

	/// <summary>
	/// The path to the directory where your .csproj file resides in your c# project.
	/// </summary>
	public string DotnetProjectDirectory { get; init; } = default!;

	public TypeMappingSettings TypeMappings { get; set; } = default!;
}