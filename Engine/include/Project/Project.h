#pragma once
#include <Core.h>
#include <Memory/Memory.h>
#include <Filesystem.h>

namespace ge {
	struct ProjectConfig {
		GEString name;
		filesystem::Path assetDirectory = "Assets";
		filesystem::Path cacheDirectory = "Cache";
		filesystem::Path configDirectory = "Config";
		filesystem::Path binariesDirectory = "Binaries";
		filesystem::Path intermediatesDirectory = "Intermediates";

		filesystem::Path scriptModulePath;
		filesystem::Path defaultScenePath = assetDirectory / "default.scene";
		filesystem::Path assetRegistryPath = (cacheDirectory / "assetRegistry.bin");
		filesystem::Path assetPakPath = (binariesDirectory / "assets.pak");
		filesystem::Path assetManifestPath = (binariesDirectory / "assets_manifest.bin");
	};
	// GLASS_ENGINE_DIR env

	class Project : public mem::RefCounted {
	public:
		static const filesystem::Path& GetProjectDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return _activeProject->_projectDirectory; }
		static filesystem::Path GetAssetDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.assetDirectory; }
		static filesystem::Path GetCacheDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.cacheDirectory; }
		static filesystem::Path GetConfigDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.configDirectory; }
		static filesystem::Path GetBinariesDirectory() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.binariesDirectory; }
		static filesystem::Path GetAssetRegistryPath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.assetRegistryPath; }
		static filesystem::Path GetScriptModulePath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.scriptModulePath; }
		static filesystem::Path GetAssetPakPath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.assetPakPath; }
		static filesystem::Path GetAssetManifestPath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.assetManifestPath; }
		static filesystem::Path GetDefaultScenePath() noexcept { GE_ASSERT(_activeProject, "No active project!"); return GetProjectDirectory() / _activeProject->_config.defaultScenePath; }
		[[nodiscard]] ProjectConfig& Config() { return _config; }
		static mem::Ref<Project> New(const GEString& name, const filesystem::Path& dir);
		static mem::Ref<Project> Load(const filesystem::Path& path);
		static mem::Ref<Project> GetActive() { return _activeProject; }
		static bool SaveActive();
	private:
		ProjectConfig _config;
		filesystem::Path _projectDirectory;
		filesystem::Path _projectPath;
		static mem::Ref<Project> _activeProject;
	};
}