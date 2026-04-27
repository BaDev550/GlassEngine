#pragma once
#include "GlassEngine/Core/Core.h"

namespace ge {
	struct ProjectConfig {
		GEString name;
		std::filesystem::path defaultScenePath = "default.scene";
		std::filesystem::path assetDirectory = "Assets";
		std::filesystem::path cacheDirectory = "Cache";
		std::filesystem::path configDirectory = "Config";
		std::filesystem::path binariesDirectory = "Binaries";
		std::filesystem::path intermediatesDirectory = "Intermediates";

		std::filesystem::path scriptModulePath;
		std::filesystem::path assetRegistryPath = cacheDirectory / "assetRegistry.bin";
		std::filesystem::path assetPakPath = binariesDirectory / "assets.pak";
		std::filesystem::path assetManifestPath = binariesDirectory / "assets_manifest.bin";
	};
	// GLASS_ENGINE_DIR env

	class Project : public mem::RefCounted {
	public:
		static const std::filesystem::path& GetProjectDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return _activeProject->_projectDirectory; }
		static std::filesystem::path GetAssetDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.assetDirectory; }
		static std::filesystem::path GetCacheDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.cacheDirectory; }
		static std::filesystem::path GetConfigDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.configDirectory; }
		static std::filesystem::path GetBinariesDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.binariesDirectory; }
		static std::filesystem::path GetAssetRegistryPath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetCacheDirectory() / _activeProject->_config.assetRegistryPath; }
		static std::filesystem::path GetScriptModulePath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetBinariesDirectory() / _activeProject->_config.scriptModulePath; }
		static std::filesystem::path GetAssetPakPath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetBinariesDirectory() / _activeProject->_config.assetPakPath; }
		static std::filesystem::path GetAssetManifestPath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetBinariesDirectory() / _activeProject->_config.assetManifestPath; }
		[[nodiscard]] ProjectConfig& Config() { return _config; }
		static mem::Ref<Project> New();
		static mem::Ref<Project> Load(const std::filesystem::path& path);
		static mem::Ref<Project> GetActive() { return _activeProject; }
		static bool SaveActive();
	private:
		ProjectConfig _config;
		std::filesystem::path _projectDirectory;
		std::filesystem::path _projectPath;
		static mem::Ref<Project> _activeProject;
	};
}