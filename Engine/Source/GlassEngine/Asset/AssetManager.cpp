#include "gepch.h"
#include "AssetManager.h"
#include "GlassEngine/Project/Project.h"
#include "GlassEngine/Core/Application.h"

namespace ge {
	mem::Ref<AssetManagerBase> AssetManager::_assetManager = nullptr;
	EditorAssetManager* AssetManager::_editorAssetManagerInstance = nullptr;
	RuntimeAssetManager* AssetManager::_runtimeAssetManagerInstance = nullptr;

	void AssetManager::Init(Project* project) {
		ApplicationMode mode = Application::Get()->GetSpecs().mode;
		switch (mode)
		{
		case ge::ApplicationMode::Editor: {
			_assetManager = mem::Ref<EditorAssetManager>::Create();
			_editorAssetManagerInstance = _assetManager.Cast<EditorAssetManager>().Get();
			break;
		}
		case ge::ApplicationMode::Runtime: {
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

	void AssetManager::Editor_CompileIntoPakFile(const std::filesystem::path& outPath) { _editorAssetManagerInstance->CompileIntoPakFile(outPath); }
	void AssetManager::Editor_CompileIntoManifest(const std::filesystem::path& outPath) { _editorAssetManagerInstance->CompileIntoManifest(outPath); }
	void AssetManager::Editor_CookAssets(const std::filesystem::path& outPath) { _editorAssetManagerInstance->CookAssets(outPath); }
	[[nodiscard]] AssetHandle AssetManager::Editor_ImportAsset(const ImportAssetData& asset, std::filesystem::path sourcePath, std::filesystem::path targetPath) {
		return _editorAssetManagerInstance->ImportAsset(asset, sourcePath, targetPath);
	}
	[[nodiscard]] AssetHandle AssetManager::Editor_CreateAsset(const std::filesystem::path& targetPath, mem::Ref<Asset> asset) {
		return _editorAssetManagerInstance->CreateAsset(targetPath, asset);
	}
}