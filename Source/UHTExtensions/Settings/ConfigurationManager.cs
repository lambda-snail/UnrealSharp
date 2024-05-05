// using System.IO;
// using System.Threading.Tasks;
// using VYaml.Serialization;
//
// namespace LambdaSnail.UnrealSharp.UHT.Extensions.Settings;
//
// public class ConfigurationManager<TConfig> where TConfig : UnrealSharpConfiguration
// {
// 	public static readonly string ConfigFileName = "unrealsharp.config.yml";
//
// 	public static ValueTask<TConfig?> GetConfigurationIfExists(string projectPath)
// 	{
// 		var configPath = Path.Combine(projectPath, ConfigFileName);
//
// 		if (!File.Exists(configPath))
// 		{
// 			return ValueTask.FromResult<TConfig?>(null);
// 		}
// 		
// 		using var stream = File.OpenRead(configPath);
// 		return YamlSerializer.DeserializeAsync<TConfig?>(stream);
// 	}
// }