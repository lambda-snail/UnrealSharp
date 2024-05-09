﻿using System.Collections.Generic;
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
public partial class UnrealSharpConfiguration
{
	public NamespaceSettings NamespaceSettings { get; init; } = default!;

	/// <summary>
	/// The path to the directory where your .csproj file resides in your c# project.
	/// </summary>
	public string DotnetProjectDirectory { get; init; } = default!;
}