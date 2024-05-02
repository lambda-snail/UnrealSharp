using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text.Json;
using System.Text.Json.Serialization;
using EpicGames.Core;
using EpicGames.UHT.Tables;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;
using UnrealBuildTool;

[UnrealHeaderTool]
public static class Exporter
{
	[UhtExporter(Name = "TestExporter", ModuleName = "UHTExtensions", Options = UhtExporterOptions.Default)]
	public static void Generate(IUhtExportFactory factory)
	{
		// Process parsed code via factory.Session here.
		factory.Session.LogInfo("TestExporter executed!");

		//factory.Session.SortedHeaderFiles[0].HeaderFile.Children

		string DotnetReadOnlySpecifier = "DotnetReadOnly";
		string DotnetReadWriteSpecifier = "DotnetReadWrite";

		var readOnlyKey = new UhtMetaDataKey(DotnetReadOnlySpecifier);
		var readWriteKey = new UhtMetaDataKey(DotnetReadWriteSpecifier);

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
						
						List<UhtProperty> properties = new();
						foreach (var type in @class.Children)
						{
							//factory.Session.LogInfo($"## {type.SourceName}");
							//foreach (var metadata in MetadataUsageFinder.FindAllMetadataUsages (type))
							// {
							// 	factory.Session.LogInfo($"#### {metadata.Value}");
							// }
							
							if (type is UhtProperty uhtProperty)
							{
								UhtMetaDataKey? foundKey = readOnlyKey;
								string? val = null;
								bool? found = uhtProperty.MetaData.Dictionary?.TryGetValue(readOnlyKey, out val);
								if (found is not true)
								{
									foundKey = readWriteKey;
									found = uhtProperty.MetaData.Dictionary?.TryGetValue(readWriteKey, out val);
								}

								if (found != true || val is null)
								{
									continue;
								}

								properties.Add(uhtProperty);
								
								factory.Session.LogInfo(
									$"# {@class.SourceName}: {foundKey} - {val}, TypeIndex: {uhtProperty.TypeIndex}");

								//borrower.StringBuilder.AppendPropertyText(property, UhtPropertyTextType.ExportMember);
								// property.AppendText(borrower.StringBuilder, UhtPropertyTextType.ExportMember); // Output property type in c++
								//borrower.StringBuilder.AppendLine(property.Setter);
							}

							// Generate extern "C" bindings here instead, as well as C# code
							// if (properties.Count > 0)
							// {
							// 	using BorrowStringBuilder borrower = new(StringBuilderCache.Big);
       //
							// 	borrower.StringBuilder.AppendLine("#pragma once");
							// 	borrower.StringBuilder.AppendLine("namespace LambdaSnail::UnrealSharp {");
							// 	
							// 	borrower.StringBuilder.Append("class ");
							// 	borrower.StringBuilder.Append(@class.GetDisplayNameText());
							// 	borrower.StringBuilder.AppendLine("_Registrators {");
							// 	borrower.StringBuilder.AppendLine("public:");
							// 	borrower.StringBuilder.Append("void RegisterProperties(LambdaSnail::UnrealSharp::ActorHandle Handle,");
       //
							// 	borrower.StringBuilder.Append(@class);
							// 	
							// 	borrower.StringBuilder.AppendLine("* ClassInstanceParameter) {");
							// 	borrower.StringBuilder.AppendLine(
							// 		"UUnrealSharpSubsystem* UnrealSharpSubsystem = ClassInstanceParameter->GetWorld()->GetGameInstance()->GetSubsystem<UUnrealSharpSubsystem>();"
							// 	);
							// 	
							// 	foreach (var property in properties)
							// 	{
							// 		// RegisterNumericProperty(LambdaSnail::UnrealSharp::ActorHandle Handle, UNumericProperty* Property)
							// 		borrower.StringBuilder.Append("UnrealSharpSubsystem->RegisterNumericProperty<");
							// 		property.AppendText(borrower.StringBuilder, UhtPropertyTextType.ExportMember); // Output property type in c++
							// 		borrower.StringBuilder.Append(">(Handle,\"");
							// 		
       //                              borrower.StringBuilder.Append(property.SourceName);
							// 		
       //                              borrower.StringBuilder.AppendLine("\");");
							// 	}
							// 	
							// 	borrower.StringBuilder.AppendLine("}");
							// 	borrower.StringBuilder.AppendLine("};"); // Class
							// 	borrower.StringBuilder.AppendLine("}");  // Namespace
							// 	
							// 	string fullPath = Path.Combine(@class.Package.Module.OutputDirectory, @class.GetDisplayNameText() + ".dotnetintegration.g.h");
							// 	factory.CommitOutput(fullPath, borrower.StringBuilder.ToString());
							// 	factory.Session.LogInfo($"Exported file {fullPath}");
							// }
							

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

	private static void GenerateCodeForClass(IUhtExportFactory factory, UhtClass @class)
	{
	}
}
