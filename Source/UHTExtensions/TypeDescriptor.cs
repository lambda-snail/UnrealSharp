namespace LambdaSnail.UnrealSharp.UHT.Extensions;

public class TypeDescriptor
{
	/// <summary>
	/// The type name in Unreal/C++.
	/// </summary>
	public string UnrealTypeName { get; set; } = default!;
	
	/// <summary>
	/// The name of the type, without namespace qualification.
	/// </summary>
	public string DotnetTypeName { get; set; } = default!;
	
	/// <summary>
	/// The namespace that contains the definition of a type in dotnet.
	/// </summary>
	public string DotnetNamespace { get; set; } = default!;
	
	public bool IsPrimitive { get; set; }

	/// <summary>
	/// Returns the type name qualified by the namespace.
	/// </summary>
	public string GetFqnDotnet()
	{
		return DotnetNamespace + "." + DotnetTypeName;
	}
}