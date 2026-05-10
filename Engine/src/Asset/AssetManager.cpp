#include "Asset/AssetManager.h"
#include "Project/Project.h"

namespace ge {
	mem::Ref<AssetManagerBase> AssetManager::_assetManager = nullptr;
	EditorAssetManager* AssetManager::_editorAssetManagerInstance = nullptr;
	RuntimeAssetManager* AssetManager::_runtimeAssetManagerInstance = nullptr;

	filesystem::Path AssetManagerBase::TryToGetGassetFile(filesystem::Path sourcePath) {
		filesystem::Path originalPath = sourcePath;
		filesystem::Path gassetPath = sourcePath.ReplaceExtension(GE_ASSET_EXTENSION);
		if (filesystem::FileSystem::Exists(gassetPath) && AssetInRegistry(GetMetadata(gassetPath).handle))
			return gassetPath;
		return originalPath;
	}

	void AssetManager::Init(Project* project) {
		EngineMode mode = Engine::Get().GetEngineSpecs().mode;
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
	[[nodiscard]] AssetMetadata AssetManager::GetMetadata(const filesystem::Path& path) { return _assetManager->GetMetadata(path); }

	bool AssetManager::AssetInRegistry(AssetHandle handle) {
		return _assetManager->AssetInRegistry(handle);
	}

	void AssetManager::Editor_RegisterDependency(AssetHandle handle, AssetHandle dependency) {
		_editorAssetManagerInstance->RegisterDependency(handle, dependency);
	}

	void AssetManager::Editor_CompileIntoPakFile(const filesystem::Path& outPath) { _editorAssetManagerInstance->CompileIntoPakFile(outPath); }
	void AssetManager::Editor_CompileIntoManifest(const filesystem::Path& outPath) { _editorAssetManagerInstance->CompileIntoManifest(outPath); }
	void AssetManager::Editor_CookAssets(const filesystem::Path& outPath) { _editorAssetManagerInstance->CookAssets(outPath); }
	void AssetManager::Editor_OnAssetDeleted(AssetHandle handle) { _editorAssetManagerInstance->OnAssetDeleted(handle); }
	AssetMap AssetManager::Editor_GetLoadedAssetsWithType(AssetType type) { return _editorAssetManagerInstance->GetLoadedAssetsWithType(type); }

	[[nodiscard]] AssetHandle AssetManager::Editor_ImportAsset(const ImportAssetData& asset, filesystem::Path sourcePath, filesystem::Path targetPath) {
		return _editorAssetManagerInstance->ImportAsset(asset, sourcePath, targetPath);
	}
	[[nodiscard]] AssetHandle AssetManager::Editor_CreateAsset(const filesystem::Path& targetPath, mem::Ref<Asset> asset) {
		return _editorAssetManagerInstance->CreateAsset(targetPath, asset);
	}
}