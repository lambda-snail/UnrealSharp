using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using EpicGames.UHT.Tables;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;

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
	
	[UhtExporter(Name = "TestExporter", ModuleName = "UHTExtensions", Options = UhtExporterOptions.Default)]
	public static void Generate(IUhtExportFactory factory)
	{
		// Process parsed code via factory.Session here.
		factory.Session.LogInfo("TestExporter executed!");

		//factory.Session.SortedHeaderFiles[0].HeaderFile.Children
		
		try
		{
			foreach (var package in factory.Session.Packages)
			{
				// if (!package.SourceName.Contains("DotnetIntegration"))
				// {
				// 	continue;
				// }

				// foreach (var metadataUsage in MetadataUsageFinder.FindAllMetadataUsages(package))
				// {
				// 	factory.Session.LogInfo($"{metadataUsage.Key}: {metadataUsage.Value} - {metadataUsage.Tag}");
				// }

				// Header files
				// foreach (var type in package.Children)
				// {
				// 	factory.Session.LogInfo(type.SourceName);
				// }

				// Classes
				// foreach (var hf in package.Children)
				// {
				// 	foreach (var type in hf.Children)
				// 	{
				// 		factory.Session.LogInfo(type.SourceName);
				// 	}
				// }

				foreach (var header in package.Children)
				{
					foreach (var @class in header.Children)
					{
						//factory.Session.LogInfo($"# {@class.SourceName}");
						
						List<PropertyDescriptor> properties = new();
						foreach (var type in @class.Children)
						{
							//factory.Session.LogInfo($"## {type.SourceName}");
							//foreach (var metadata in MetadataUsageFinder.FindAllMetadataUsages (type))
							// {
							// 	factory.Session.LogInfo($"#### {metadata.Value}");
							// }
							
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

								//borrower.StringBuilder.AppendPropertyText(property, UhtPropertyTextType.ExportMember);
								// property.AppendText(borrower.StringBuilder, UhtPropertyTextType.ExportMember); // Output property type in c++
								//borrower.StringBuilder.AppendLine(property.Setter);
							}

							// TODO: Generate extern "C" bindings here instead, as well as C# code
							if (properties.Count > 0)
							{
								using BorrowStringBuilder borrower = new(StringBuilderCache.Big);
       
								borrower.StringBuilder.AppendLine("#pragma once");
								borrower.StringBuilder.AppendLine("namespace LambdaSnail::UnrealSharp {");
								
								// borrower.StringBuilder.Append("class ");
								// borrower.StringBuilder.Append(@class.GetDisplayNameText());
								// borrower.StringBuilder.AppendLine("_DotnetBindings {");
								// borrower.StringBuilder.AppendLine("public:");

								foreach (PropertyDescriptor descriptor in properties)
								{
									GenerateBindingsForProperty(descriptor, borrower, @class);
								}
								
								// extern "C" __declspec(dllexport) inline void GetRotation(AActor const* Actor, void* Rotator)
								// {
								// 	FVector* Vec = static_cast<FVector*>(Rotator);
								// 	*Vec = Actor->GetActorRotation().Euler();
								// }
								
								// borrower.StringBuilder.AppendLine("* ClassInstanceParameter) {");
								// borrower.StringBuilder.AppendLine(
								// 	"UUnrealSharpSubsystem* UnrealSharpSubsystem = ClassInstanceParameter->GetWorld()->GetGameInstance()->GetSubsystem<UUnrealSharpSubsystem>();"
								// );
								//
								// foreach (var property in properties)
								// {
								// 	// RegisterNumericProperty(LambdaSnail::UnrealSharp::ActorHandle Handle, UNumericProperty* Property)
								// 	borrower.StringBuilder.Append("UnrealSharpSubsystem->RegisterNumericProperty<");
								// 	property.AppendText(borrower.StringBuilder, UhtPropertyTextType.ExportMember); // Output property type in c++
								// 	borrower.StringBuilder.Append(">(Handle,\"");
								// 	
        //                             borrower.StringBuilder.Append(property.SourceName);
								// 	
        //                             borrower.StringBuilder.AppendLine("\");");
								// }
								
								borrower.StringBuilder.AppendLine("}");  // Namespace
								
								string fullPath = Path.Combine(@class.Package.Module.OutputDirectory, @class.GetDisplayNameText() + ".dotnetintegration.g.h");
								factory.CommitOutput(fullPath, borrower.StringBuilder.ToString());
								factory.Session.LogInfo($"Exported file {fullPath}");
							}
							

							// foreach (var (key, value) in type.MetaData.Dictionary!)
							// {
							// 	factory.Session.LogInfo($"#### {key} - {value}");
							// }
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
		borrower.StringBuilder.Append("extern \"C\" __declspec(dllexport) inline void Get_");
		borrower.StringBuilder.Append(property.GetDisplayNameText());
		borrower.StringBuilder.Append("(");
		borrower.StringBuilder.Append(@class.GetDisplayNameText());
		borrower.StringBuilder.AppendLine(" const* Instance, void* Parameter) {");
		
		borrower.StringBuilder.Append("auto* TypedPtr = static_cast<");
		property.AppendText(borrower.StringBuilder, UhtPropertyTextType.ExportMember);
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
				borrower.StringBuilder.Append("*TypedPtr = Val;");
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
