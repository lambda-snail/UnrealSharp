using System.Collections.Concurrent;
using System.IO;
using System.Threading.Tasks;
using VYaml.Serialization;

namespace LambdaSnail.UnrealSharp.UHT.Extensions.Settings;

public class ConfigurationManager : UnrealSharpConfiguration
{
	public static readonly string ConfigFileName = "unrealsharp.config.yml";

	private static readonly ConcurrentDictionary<string, UnrealSharpConfiguration> ProjectConfigTable;
	
	static ConfigurationManager()
	{
		ProjectConfigTable = new();
	}
	
	public static async ValueTask<UnrealSharpConfiguration?> GetConfigurationIfExists(string projectPath)
	{
		if (ProjectConfigTable.TryGetValue(projectPath, out var exists))
		{
			return exists;
		}
		
		var configPath = Path.Combine(projectPath, ConfigFileName);
		if (!File.Exists(configPath))
		{
			return null;
		}
		
		await using var stream = File.OpenRead(configPath);
		var config = await YamlSerializer.DeserializeAsync<UnrealSharpConfiguration>(stream);
		ProjectConfigTable.TryAdd(projectPath, config);
		return config;
	}
}