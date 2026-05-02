#include "gepch.h"
#include "AssetManager.h"
#include "GlassEngine/Project/Project.h"
#include "GlassEngine/Core/Engine.h"

namespace ge {
	mem::Ref<AssetManagerBase> AssetManager::_assetManager = nullptr;
	EditorAssetManager* AssetManager::_editorAssetManagerInstance = nullptr;
	RuntimeAssetManager* AssetManager::_runtimeAssetManagerInstance = nullptr;

	std::filesystem::path AssetManagerBase::TryToGetGassetFile(std::filesystem::path sourcePath) {
		std::filesystem::path originalPath = sourcePath;
		std::filesystem::path gassetPath = sourcePath.replace_extension(GE_ASSET_EXTENSION);
		if (std::filesystem::exists(gassetPath) && AssetInRegistry(GetMetadata(gassetPath).handle))
			return gassetPath;
		return originalPath;
	}

	void AssetManager::Init(Project* project) {
		EngineMode mode = Engine::Get().GetSpecs().mode;
		switch (mode)
		{
		case EngineMode::Editor: {
			_assetManager = mem::Ref<EditorAssetManager>::Create(project->GetAssetRegistryPath());
			_editorAssetManagerInstance = _assetManager.Cast<EditorAssetManager>().Get();
			break;
		}
		case EngineMode::Runtime: {
			_assetManager = mem::Ref<RuntimeAssetManager>::Create(project->GetAssetPakPath(), project->GetAssetManifestPath());
			_runtimeAssetManagerInstance = _assetManager.Cast<RuntimeAssetManager>().Get();
			break;
		}
		}
		GE_CORE_INFO("Asset manager created");
	}
	void AssetManager::Destroy() {
		_assetManager = nullptr;
		_editorAssetManagerInstance = nullptr;
		_runtimeAssetManagerInstance = nullptr;
	}

	[[nodiscard]] AssetMetadata AssetManager::GetMetadata(AssetHandle handle) { return _assetManager->GetMetadata(handle); }
	[[nodiscard]] AssetMetadata AssetManager::GetMetadata(const std::filesystem::path& path) { return _assetManager->GetMetadata(path); }

	void AssetManager::Editor_RegisterDependency(AssetHandle handle, AssetHandle dependency) {
		_editorAssetManagerInstance->RegisterDependency(handle, dependency);
	}

	void AssetManager::Editor_CompileIntoPakFile(const std::filesystem::path& outPath) { _editorAssetManagerInstance->CompileIntoPakFile(outPath); }
	void AssetManager::Editor_CompileIntoManifest(const std::filesystem::path& outPath) { _editorAssetManagerInstance->CompileIntoManifest(outPath); }
	void AssetManager::Editor_CookAssets(const std::filesystem::path& outPath) { _editorAssetManagerInstance->CookAssets(outPath); }
	AssetMap AssetManager::Editor_GetLoadedAssetsWithType(AssetType type) { return _editorAssetManagerInstance->GetLoadedAssetsWithType(type); }

	[[nodiscard]] AssetHandle AssetManager::Editor_ImportAsset(const ImportAssetData& asset, std::filesystem::path sourcePath, std::filesystem::path targetPath) {
		return _editorAssetManagerInstance->ImportAsset(asset, sourcePath, targetPath);
	}
	[[nodiscard]] AssetHandle AssetManager::Editor_CreateAsset(const std::filesystem::path& targetPath, mem::Ref<Asset> asset) {
		return _editorAssetManagerInstance->CreateAsset(targetPath, asset);
	}
}