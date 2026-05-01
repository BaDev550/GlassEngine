#include "gepch.h"
#include "ProjectSerializer.h"
#include "GlassEngine/Scene/SceneSerializer.h"
#include <cstdlib>

#include <yaml-cpp/yaml.h>

namespace ge {
	ProjectSerializer::ProjectSerializer(mem::Ref<Project> project) : _Project(project) { }

	bool ProjectSerializer::CreateProjectDirectories(const std::string& name, const std::filesystem::path& dir) {
		ProjectConfig config = ProjectConfig();

		config.name = name;
		config.scriptModulePath = config.binariesDirectory / (name + ".dll");
		const char* envPath = std::getenv("GLASS_ENGINE_DIR");
		if (!envPath) {
			GE_CORE_ERROR("Environment variable GLASS_ENGINE_DIR not found!");
			return false;
		}

		std::filesystem::path engineDir = envPath;
		std::filesystem::path projectDir = dir / name;
		std::filesystem::path projectFile = projectDir / (name + ".geproj");

		std::filesystem::path configDir = projectDir / config.configDirectory;
		std::filesystem::path cacheDir = projectDir / config.cacheDirectory;
		std::filesystem::path intermediatesDir = projectDir / config.intermediatesDirectory;

		std::filesystem::path cmakePath = projectDir / "CMakeLists.txt";
		std::filesystem::path cacheAbsDir = std::filesystem::absolute(intermediatesDir);

		mem::Ref<Scene> scene = mem::Ref<Scene>::Create(config.defaultScenePath.stem().string());
		SceneSerializer scene_serializer(scene);

		std::string command;
#ifdef _WIN32
		command = "cd /D \"" + cacheAbsDir.string() + "\" && " + "cmake ..";
#else
		command = "cd \"" + cacheAbsDir.generic_string() + "\" && \"" + cmakePath.generic_string() + "\"";
#endif
		std::filesystem::create_directories(projectDir);
		std::filesystem::create_directories(cacheDir);
		std::filesystem::create_directories(cacheDir / "Logs");
		std::filesystem::create_directories(configDir);
		std::filesystem::create_directories(projectDir / config.assetDirectory);
		std::filesystem::create_directories(projectDir / config.binariesDirectory);
		std::filesystem::create_directories(intermediatesDir);

		if (CreateCMakeFile(cmakePath, config)) {
			SerializeWithConfig(projectFile, config);

			scene_serializer.Serialize((projectDir / config.defaultScenePath).string());

			std::ofstream defaultCS((projectDir / config.assetDirectory) / "Test.cs");
			std::ofstream renderConfig(configDir / "render.config");
			std::ofstream defaultsConfig(configDir / "project.config");
			
			std::system(command.c_str());
			return true;
		}
		else {
			GE_CORE_ERROR("Failed to create cmake file!");
			return false;
		}
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& path)
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

		std::ofstream fout(path);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::SerializeWithConfig(const std::filesystem::path& path, const ProjectConfig& config)
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

		std::ofstream fout(path);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& path)
	{
		auto& config = _Project->Config();
		YAML::Node data;
		try {
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e) {
			GE_CORE_ERROR("Failed to load project: {}", path.string());
		}

		auto projectNode = data["Project"];
		if (!projectNode)
			return false;

		config.name = projectNode["Name"].as<std::string>();
		config.cacheDirectory = projectNode["CacheDirectory"].as<std::string>();
		config.configDirectory = projectNode["ConfigDirectory"].as<std::string>();
		config.binariesDirectory = projectNode["BinariesDirectory"].as<std::string>();
		config.intermediatesDirectory = projectNode["IntermediatesDirectory"].as<std::string>();
		config.assetDirectory = projectNode["AssetDirectory"].as<std::string>();
		config.scriptModulePath = projectNode["ScriptModulePath"].as<std::string>();
		config.assetRegistryPath = projectNode["AssetRegistryPath"].as<std::string>();
		config.assetPakPath = projectNode["AssetPakPath"].as<std::string>();
		config.assetManifestPath = projectNode["AssetManifestPath"].as<std::string>();
		return true;
	}

	bool ProjectSerializer::CreateCMakeFile(const std::filesystem::path& cmakePath, const ProjectConfig& config)
	{
		const char* env = std::getenv("GLASS_ENGINE_DIR");
		if (!env) {
			GE_CORE_ERROR("GLASS_ENGINE_DIR environment variable is not set!");
			return false;
		}
		std::filesystem::path engineDir = env;
		std::ofstream cmake(cmakePath);

		if (!cmake.is_open()) {
			GE_CORE_ERROR("Failed to open CMakeLists.txt for writing!");
			return false;
		}

		std::string enginePathStr = engineDir.generic_string();
		std::string binDir = "${CMAKE_SOURCE_DIR}/" + config.binariesDirectory.generic_string();

		cmake << "cmake_minimum_required(VERSION 3.23)\n";
		cmake << "project(" << config.name << " LANGUAGES CSharp)\n\n";

		cmake << "set(ENGINE_DIR \"" << enginePathStr << "\")\n\n";

		cmake << "set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY \"" << binDir << "\")\n";
		cmake << "set(CMAKE_LIBRARY_OUTPUT_DIRECTORY \"" << binDir << "\")\n";
		cmake << "set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \"" << binDir << "\")\n\n";
		cmake << "foreach(OUTPUTCONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)\n";
		cmake << "    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)\n";
		cmake << "    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} \"" << binDir << "\")\n";
		cmake << "    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} \"" << binDir << "\")\n";
		cmake << "    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} \"" << binDir << "\")\n";
		cmake << "endforeach()\n\n";

		cmake << "file(GLOB_RECURSE SOURCE_FILES \"Assets/*.cs\")\n\n";
		cmake << "add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})\n\n";

		cmake << "set_target_properties(${PROJECT_NAME} PROPERTIES\n";
		cmake << "    DOTNET_SDK \"Microsoft.NET.Sdk\"\n";
		cmake << "    DOTNET_TARGET_FRAMEWORK \"net472\"\n";
		cmake << ")\n\n";

		//cmake << "set_property(TARGET ${PROJECT_NAME} PROPERTY\n";
		//cmake << "    VS_DOTNET_REFERENCE_ScriptCore \"${ENGINE_DIR}/bin/Release/ScriptCore.dll\"\n";
		//cmake << ")\n";

		cmake.close();
		return true;
	}
}