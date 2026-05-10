#include "Engine.h"
#include "Project/ProjectSerializer.h"
#include "Scene/SceneSerializer.h"
#include <Logger.h>
#include <cstdlib>

#include <yaml-cpp/yaml.h>

namespace YAML {
	template<>
	struct convert<ge::GEString> {
		static Node encode(const ge::GEString& rhs) {
			return Node(rhs.c_str());
		}

		static bool decode(const Node& node, ge::GEString& rhs) {
			if (!node.IsScalar())
				return false;
			rhs = node.as<std::string>().c_str();
			return true;
		}
	};
}

namespace ge {
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const ge::GEString& v) {
		out << std::string(v.c_str());
		return out;
	}

	ProjectSerializer::ProjectSerializer(mem::Ref<Project> project) : _Project(project) { }

	bool ProjectSerializer::CreateProjectDirectories(const GEString& name, const filesystem::Path& dir) {
		ProjectConfig config = ProjectConfig();

		config.name = name;
		config.scriptModulePath = config.binariesDirectory / (name + ".dll");

		filesystem::Path engineDir = GE_DIR;
		filesystem::Path projectDir = (engineDir / name);
		filesystem::Path projectFile = projectDir / (name + ".geproj");

		filesystem::Path configDir = projectDir / config.configDirectory;
		filesystem::Path cacheDir = projectDir / config.cacheDirectory;
		filesystem::Path intermediatesDir = projectDir / config.intermediatesDirectory;

		filesystem::Path cmakePath = projectDir / "CMakeLists.txt";
		filesystem::Path cacheAbsDir = filesystem::FileSystem::GetAbsolutePath(intermediatesDir);

		mem::Ref<Scene> scene = mem::Ref<Scene>::Create(config.defaultScenePath.GetFileName());
		SceneSerializer scene_serializer(scene);

		std::string command;
#ifdef _WIN32
		command = "cd /D \"" + cacheAbsDir.string() + "\" && " + "cmake ..";
#else
		command = "cd \"" + cacheAbsDir.string() + "\" && \"" + cmakePath.string() + "\"";
#endif
		filesystem::FileSystem::CreateDir(projectDir);
		filesystem::FileSystem::CreateDir(cacheDir);
		filesystem::FileSystem::CreateDir(cacheDir / "Logs");
		filesystem::FileSystem::CreateDir(configDir);
		filesystem::FileSystem::CreateDir(projectDir / config.assetDirectory);
		filesystem::FileSystem::CreateDir(projectDir / config.binariesDirectory);
		filesystem::FileSystem::CreateDir(intermediatesDir);

		if (CreateCMakeFile(cmakePath, config)) {
			SerializeWithConfig(projectFile, config);

			scene_serializer.Serialize((projectDir / config.defaultScenePath).string());

			std::ofstream defaultCS(((projectDir / config.assetDirectory) / "Test.cs").c_str());
			std::ofstream renderConfig((configDir / "render.config").c_str());
			std::ofstream defaultsConfig((configDir / "project.config").c_str());
			
			std::system(command.c_str());
			return true;
		}
		else {
			GE_CORE_ERROR("Failed to create cmake file!");
			return false;
		}
	}

	bool ProjectSerializer::Serialize(const filesystem::Path& path)
	{
		const auto& config = _Project->Config();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << config.name.c_str();
		out << YAML::Key << "CacheDirectory" << YAML::Value << config.cacheDirectory.string();
		out << YAML::Key << "ConfigDirectory" << YAML::Value << config.configDirectory.string();
		out << YAML::Key << "BinariesDirectory" << YAML::Value << config.binariesDirectory.string();
		out << YAML::Key << "IntermediatesDirectory" << YAML::Value << config.intermediatesDirectory.string();
		out << YAML::Key << "AssetDirectory" << YAML::Value << config.assetDirectory.string();
		out << YAML::Key << "AssetRegistryPath" << YAML::Value << config.assetRegistryPath.string();
		out << YAML::Key << "ScriptModulePath" << YAML::Value << config.scriptModulePath.string();
		out << YAML::Key << "AssetPakPath" << YAML::Value << config.assetPakPath.string();
		out << YAML::Key << "AssetManifestPath" << YAML::Value << config.assetManifestPath.string();
		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream fout(path.c_str());
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::SerializeWithConfig(const filesystem::Path& path, const ProjectConfig& config)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << config.name.c_str();
		out << YAML::Key << "CacheDirectory" << YAML::Value << config.cacheDirectory.string();
		out << YAML::Key << "ConfigDirectory" << YAML::Value << config.configDirectory.string();
		out << YAML::Key << "BinariesDirectory" << YAML::Value << config.binariesDirectory.string();
		out << YAML::Key << "IntermediatesDirectory" << YAML::Value << config.intermediatesDirectory.string();
		out << YAML::Key << "AssetDirectory" << YAML::Value << config.assetDirectory.string();
		out << YAML::Key << "AssetRegistryPath" << YAML::Value << config.assetRegistryPath.string();
		out << YAML::Key << "ScriptModulePath" << YAML::Value << config.scriptModulePath.string();
		out << YAML::Key << "AssetPakPath" << YAML::Value << config.assetPakPath.string();
		out << YAML::Key << "AssetManifestPath" << YAML::Value << config.assetManifestPath.string();
		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream fout(path.c_str());
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const filesystem::Path& path)
	{
		auto& config = _Project->Config();
		YAML::Node data;
		try {
			data = YAML::LoadFile(path.c_str());
		}
		catch (YAML::ParserException e) {
			GE_CORE_ERROR("Failed to load project: {}", path.string());
		}

		auto projectNode = data["Project"];
		if (!projectNode)
			return false;

		config.name = projectNode["Name"].as<GEString>();
		config.cacheDirectory = projectNode["CacheDirectory"].as<GEString>();
		config.configDirectory = projectNode["ConfigDirectory"].as<GEString>();
		config.binariesDirectory = projectNode["BinariesDirectory"].as<GEString>();
		config.intermediatesDirectory = projectNode["IntermediatesDirectory"].as<GEString>();
		config.assetDirectory = projectNode["AssetDirectory"].as<GEString>();
		config.scriptModulePath = projectNode["ScriptModulePath"].as<GEString>();
		config.assetRegistryPath = projectNode["AssetRegistryPath"].as<GEString>();
		config.assetPakPath = projectNode["AssetPakPath"].as<GEString>();
		config.assetManifestPath = projectNode["AssetManifestPath"].as<GEString>();
		return true;
	}

	bool ProjectSerializer::CreateCMakeFile(const filesystem::Path& cmakePath, const ProjectConfig& config)
	{
		return true;
	}
}