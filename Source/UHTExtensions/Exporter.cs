using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using EpicGames.UHT.Tables;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;
using LambdaSnail.UnrealSharp.UHT.Extensions;

public enum AccessMode
{
	ReadOnly,
	ReadWrite
}

public enum AccessMethod
{
	Public,
	MemberFunction,
	UnrealReflection
}

public struct AccessInformation
{
	public AccessMethod AccessMethod { get; set; }
	public string? MemberFunctionForPropertyAccess { get; set; }
}

public struct PropertyDescriptor
{
	public UhtProperty Property { get; set; }
	public AccessMode AccessMode { get; set; }
	public AccessInformation AccessInformation { get; set; }

	public string GetNameOfGetter()
	{
		return "Get" + Property.GetDisplayNameText();
	}
	
	public string GetNameOfSetter()
	{
		return "Set" + Property.GetDisplayNameText();
	}

	public void EmitPropertyTypeString(StringBuilder builder)
	{
		Property.AppendText(builder, UhtPropertyTextType.ExportMember);
	}
}

[UnrealHeaderTool]
public static class Exporter
{
	/// <summary>
	/// If this is set to true, dotnet code will only be able to read the corresponding UProperty. 
	/// </summary>
	static readonly string DotnetReadOnlySpecifier = "DotnetReadOnly";
	
	/// <summary>
	/// Allows dotnet to both read and write the corresponding UProperty.
	/// </summary>
	static readonly string DotnetReadWriteSpecifier = "DotnetReadWrite";
	
	/// <summary>
	/// Determines how the binding functions access the UProperty. The following values are allowed:
	/// - Public: The generated bindings assume that the UProperty is public
	/// - Function Name: The generated bindings will attempt to read and write the UProperty using a member function with
	///		this name.
	/// - Empty (not specified): The generated bindings will attempt to access the UProperty using the reflection system (FProperty et al.)
	/// </summary>
	static readonly string DotnetAccessSpecifier = "DotnetAccess";
	
	static readonly UhtMetaDataKey readOnlyKey = new UhtMetaDataKey(DotnetReadOnlySpecifier);
	static readonly UhtMetaDataKey readWriteKey = new UhtMetaDataKey(DotnetReadWriteSpecifier);
	static readonly UhtMetaDataKey accessSpecifierKey = new UhtMetaDataKey(DotnetAccessSpecifier);
	
	[UhtExporter(Name = "UnrealSharpExporter", ModuleName = "UHTExtensions", Options = UhtExporterOptions.Default)]
	public static void Generate(IUhtExportFactory factory)
	{
		try
		{
			ConcurrentBag<UnrealSharpOutput> outputs = new();
			List<Task?> exportTasks = new(); 
			
			foreach (var package in factory.Session.Packages)
			{
				foreach (var header in package.Children)
				{
					foreach (var @class in header.Children)
					{
						List<PropertyDescriptor> properties = new();
						foreach (var type in @class.Children)
						{
							if (type is UhtProperty uhtProperty)
							{
								AccessMode? accessMode = GetAccessModeKey(uhtProperty);
								if (accessMode is null)
								{
									continue;
								}
								
								AccessInformation accessInformation = GetAccessInformation(uhtProperty);
		
								properties.Add(new()
								{
									AccessMode = accessMode.Value,
									AccessInformation = accessInformation,
									Property = uhtProperty
								});
							}
		
							// TODO: Generate extern "C" bindings here instead, as well as C# code
							if (properties.Count > 0)
							{
								using BorrowStringBuilder borrower = new(StringBuilderCache.Big);
       
								borrower.StringBuilder.AppendLine("#pragma once");
								borrower.StringBuilder.AppendLine("namespace LambdaSnail::UnrealSharp {");
		
								foreach (PropertyDescriptor descriptor in properties)
								{
									GenerateBindingsForProperty(descriptor, borrower, @class);
								}
								
								borrower.StringBuilder.AppendLine("}");  // Namespace
								
								string fullPath = Path.Combine(@class.Package.Module.OutputDirectory, @class.GetDisplayNameText() + ".dotnetintegration.g.h");
								factory.CommitOutput(fullPath, borrower.StringBuilder.ToString());
								factory.Session.LogInfo($"Exported file {fullPath}");
		
								if (@class is UhtClass unrealClass)
								{
									borrower.StringBuilder.Clear();

									DotnetClassGenerator dotnetGenerator = new(factory);
									using BorrowStringBuilder builder = new(StringBuilderCache.Big);
									dotnetGenerator.EmitClass(unrealClass, properties, builder.StringBuilder).Wait();
								}
							}
						}
					}
				}
			}
		}
		catch (Exception e)
		{
			factory.Session.LogError(e.Message);
			factory.Session.LogError(e.StackTrace ?? string.Empty);
		}
	}

	private static void GenerateBindingsForProperty(PropertyDescriptor descriptor, BorrowStringBuilder borrower,
		UhtType @class)
	{
		UhtProperty property = descriptor.Property;

		// Getter - binding signature
		EmitMemberAccessFunction(descriptor.GetNameOfGetter(), isConstInstance:true, borrower, @class, property);
		
		borrower.StringBuilder.Append("auto* TypedPtr = static_cast<");
		descriptor.EmitPropertyTypeString(borrower.StringBuilder);
		borrower.StringBuilder.AppendLine("*>(Parameter);");
		
		switch (descriptor.AccessInformation.AccessMethod)
		{
			case AccessMethod.Public:
				borrower.StringBuilder.Append("*TypedPtr = Instance->");
				borrower.StringBuilder.Append(property.GetDisplayNameText());
				borrower.StringBuilder.AppendLine(";");
				break;
			case AccessMethod.UnrealReflection:
				//FProperty* Property2 = XXX::StaticClass()->FindPropertyByName("FloatProp");
				borrower.StringBuilder.Append("static FProperty* Property = ");
				borrower.StringBuilder.Append(@class.GetDisplayNameText());
				borrower.StringBuilder.Append("::StaticClass()->FindPropertyByName(\"");
				borrower.StringBuilder.Append(property.GetDisplayNameText());
				borrower.StringBuilder.AppendLine("\");");
				
				//example: int32 P; Property->GetValue_InContainer(this, &P); *TypedPtr = Val;
				property.AppendText(borrower.StringBuilder, UhtPropertyTextType.ExportMember);
				borrower.StringBuilder.Append(" Val;");
				borrower.StringBuilder.AppendLine("Property->GetValue_InContainer(Instance, &Val);");
				borrower.StringBuilder.AppendLine("*TypedPtr = Val;");
				break;
			case AccessMethod.MemberFunction:
				borrower.StringBuilder.Append("*TypedPtr = Instance->Get"); // Get + Function Name
				borrower.StringBuilder.Append(descriptor.AccessInformation.MemberFunctionForPropertyAccess);
				borrower.StringBuilder.AppendLine("();");
				break;
			default:
				throw new InvalidOperationException($"Unknown access method: ${descriptor.AccessInformation.AccessMethod}");
		}

		borrower.StringBuilder.AppendLine("}"); // End of getter
		borrower.StringBuilder.AppendLine();
		
		if (descriptor.AccessMode != AccessMode.ReadWrite)
		{
			return;
		}
		
		// Setter - binding signature
		EmitMemberAccessFunction(descriptor.GetNameOfSetter(), isConstInstance:false, borrower, @class, property);

		switch (descriptor.AccessInformation.AccessMethod)
		{
			case AccessMethod.Public:
				// Instance->Property = *static_cast<Type*>(Parameter);
				borrower.StringBuilder.Append("Instance->");
				borrower.StringBuilder.Append(property.GetDisplayNameText());
				borrower.StringBuilder.Append(" = *static_cast<");
				descriptor.EmitPropertyTypeString(borrower.StringBuilder);
				borrower.StringBuilder.AppendLine("*>(Parameter);");
				break;
			case AccessMethod.UnrealReflection:
				// static FProperty* Property = ClassName::StaticClass()->FindPropertyByName("<PropertyName>");
				borrower.StringBuilder.Append("static FProperty* Property = ");
				borrower.StringBuilder.Append(@class.GetDisplayNameText());
				borrower.StringBuilder.Append("::StaticClass()->FindPropertyByName(\"");
				borrower.StringBuilder.Append(property.GetDisplayNameText());
				borrower.StringBuilder.AppendLine("\");");
				
				// Property->SetValue_InContainer(static_cast<void*>(Instance), static_cast<PropertyType*>(Parameter));
				borrower.StringBuilder.Append("Property->SetValue_InContainer(static_cast<void*>(Instance), static_cast<");
				descriptor.EmitPropertyTypeString(borrower.StringBuilder);
				borrower.StringBuilder.AppendLine("*>(Parameter));");
				break;
			case AccessMethod.MemberFunction:
				// Instance->FunctionName(*static_cast<Type*>(Parameter));
				borrower.StringBuilder.Append("Instance->Set");
				borrower.StringBuilder.Append(descriptor.AccessInformation.MemberFunctionForPropertyAccess);
				borrower.StringBuilder.Append("(*static_cast<");
				descriptor.EmitPropertyTypeString(borrower.StringBuilder);
				borrower.StringBuilder.AppendLine("*>(Parameter));");
				break;
			default:
				throw new InvalidOperationException($"Unknown access method: ${descriptor.AccessInformation.AccessMethod}");
		}
	
		borrower.StringBuilder.AppendLine("}"); // End of setter
		borrower.StringBuilder.AppendLine();
	}
	
	private static void EmitMemberAccessFunction(string accessFunctionName, bool isConstInstance, BorrowStringBuilder borrower, UhtType @class, UhtProperty property)
	{
		borrower.StringBuilder.Append("extern \"C\" __declspec(dllexport) inline void ");
		borrower.StringBuilder.Append(accessFunctionName);
		borrower.StringBuilder.Append("(");
		borrower.StringBuilder.Append(@class.GetDisplayNameText());
		if (isConstInstance)
		{
			borrower.StringBuilder.Append(" const");
		}
		borrower.StringBuilder.AppendLine("* Instance, void* Parameter) {");
	}

	private static AccessMode? GetAccessModeKey(UhtProperty uhtProperty)
	{
		bool? found = uhtProperty.MetaData.Dictionary?.ContainsKey(readOnlyKey);
		if (found is true)
		{
			return AccessMode.ReadOnly;
		}

		found = uhtProperty.MetaData.Dictionary?.ContainsKey(readWriteKey);
		if (found is true)
		{
			return AccessMode.ReadWrite;
		}

		return null;
	}

	private static AccessInformation GetAccessInformation(UhtProperty uhtProperty)
	{
		string? accessMethod = null;
		bool? found = uhtProperty.MetaData.Dictionary?.TryGetValue(accessSpecifierKey, out accessMethod);
		if (found == true)
		{
			Console.WriteLine(accessMethod);
			AccessMethod method = accessMethod switch
			{
				"Public" => AccessMethod.Public,
				null or "" or "Reflection" => AccessMethod.UnrealReflection,
				_ => AccessMethod.MemberFunction
			};
			
			return new() { AccessMethod = method, MemberFunctionForPropertyAccess = accessMethod };
		}

		return new() { AccessMethod = AccessMethod.Public, MemberFunctionForPropertyAccess = "" };
	}

	private static void GenerateCodeForClass(IUhtExportFactory factory, UhtClass @class)
	{
	}
}
